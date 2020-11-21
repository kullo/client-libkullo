/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/sync/messagesuploader.h"

#include <functional>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/debug.h"
#include "kulloclient/codec/exceptions.h"
#include "kulloclient/codec/messagecompressor.h"
#include "kulloclient/codec/messageencryptor.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/hasher.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/daoutil.h"
#include "kulloclient/dao/deliverydao.h"
#include "kulloclient/dao/symmetrickeydao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/http/TransferProgress.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/numeric_cast.h"
#include "kulloclient/util/strings.h"

using namespace Kullo::Dao;
using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

MessagesUploader::MessagesUploader(
        const UserSettings &settings,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
        const Db::SharedSessionPtr &session,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : session_(session),
      settings_(settings),
      privKeyProvider_(privKeyProvider)
{
    msgAdder_.events.conversationAdded = forwardEvent(events.conversationAdded);
    msgAdder_.events.conversationModified = forwardEvent(events.conversationModified);
    msgAdder_.events.messageAdded = forwardEvent(events.messageAdded);
    msgAdder_.events.senderAdded = forwardEvent(events.senderAdded);

    messagesClient_.reset(new Protocol::MessagesClient(
                             *settings_.credentials.address,
                             *settings_.credentials.masterKey,
                              httpClient));
}

MessagesUploader::~MessagesUploader()
{
}

SyncOutgoingMessagesProgress MessagesUploader::initialProgress()
{
    SyncOutgoingMessagesProgress result;
    result.totalBytes =
            Dao::DraftDao::sizeOfAllSendable(session_) +
            Dao::MessageDao::sizeOfAllUndelivered(session_);
    return result;
}

void MessagesUploader::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    while (true)
    {
        auto draft = DraftDao::firstSendable(session_);
        if (!draft)
        {
            return;
        }
        if (*shouldCancel) throw SyncCanceled();

        estimatedRemaining_ =
                Dao::DraftDao::sizeOfAllSendable(session_)
                + Dao::MessageDao::sizeOfAllUndelivered(session_);
        progress_.uploadedBytes = previouslyUploaded_;
        progress_.totalBytes = previouslyUploaded_ + estimatedRemaining_;
        EMIT(events.progressed, progress_);

        std::unique_ptr<Dao::ConversationDao> conv;
        auto dateSent = DateTime::nowUtc();
        boost::optional<Protocol::MessageSent> msgSent;
        std::vector<Util::Delivery> delivery;
        try
        {
            conv = loadConversation(draft->conversationId());

            if (*shouldCancel) throw SyncCanceled();

            // encode message
            auto encodedMessage = Codec::MessageEncoder::encodeMessage(
                        settings_.credentials,
                        *draft,
                        dateSent,
                        session_);

            if (*shouldCancel) throw SyncCanceled();

            // must be calculated the same way as sizeOfAllSendable()
            auto totalSize =
                    Dao::PER_MESSAGE_OVERHEAD
                    + draft->text().size()
                    + encodedMessage.attachments.size();

            Codec::MessageCompressor().compress(encodedMessage);

            if (*shouldCancel) throw SyncCanceled();

            auto sendableMsg = makeSendableMessage(encodedMessage);
            if (!ensureSizeLimitCompliance(sendableMsg, *draft)) continue;

            if (*shouldCancel) throw SyncCanceled();

            // create and encode meta
            delivery = makeDelivery(*conv);
            auto encodedMeta = Codec::MessageEncoder::encodeMeta(
                        true,  // read
                        true,  // done
                        delivery);
            auto meta = Codec::MessageEncryptor().encryptMeta(
                        encodedMeta,
                        Dao::SymmetricKeyDao::loadKey(
                            Dao::SymmetricKeyDao::PRIVATE_DATA_KEY,
                            session_));

            if (*shouldCancel) throw SyncCanceled();

            // send message
            msgSent = messagesClient_->sendMessageToSelf(
                        sendableMsg,
                        meta,
                        [this, totalSize, shouldCancel]
                        (const Http::TransferProgress &progress)
            {
                // make sure uploadTotal has a sane value
                auto uploadTotal = std::max(
                            progress.uploadTotal,
                            progress.uploadTransferred);

                int64_t improvedEstimate;
                if (uploadTotal > 0)
                {
                    // Remove the current upload's impact on the estimate,
                    // replace by value returned by the HTTP client.
                    //
                    // Make all variables signed to ensure no subtraction is
                    // performed on unsigned ints causing an overflow.
                    improvedEstimate =
                            Util::numeric_cast<int64_t>(estimatedRemaining_.load()) -
                            Util::numeric_cast<int64_t>(totalSize) +
                            Util::numeric_cast<int64_t>(uploadTotal);
                }
                else
                {
                    improvedEstimate = estimatedRemaining_;
                }

                SyncOutgoingMessagesProgress newProgress;
                newProgress.uploadedBytes =
                        previouslyUploaded_ + progress.uploadTransferred;
                newProgress.totalBytes = previouslyUploaded_ + improvedEstimate;

                if (newProgress != progress_)
                {
                    progress_ = newProgress;
                    EMIT(events.progressed, progress_);
                }

                if (*shouldCancel) messagesClient_->cancel();
            });
        }
        catch (Protocol::Canceled&)
        {
            throw SyncCanceled();
        }
        catch (SyncCanceled&)
        {
            throw;
        }
        catch (std::exception &ex)
        {
            Log.e() << "MessagesUploader failed: " << Util::formatException(ex);

            SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);
            auto deletedIndexes = Dao::AttachmentDao::deleteAttachmentsForDraft(
                        conv->id(), session_);
            resetDraftToEditing(*draft);
            tx.commit();

            for (auto index : deletedIndexes) {
                EMIT(events.draftAttachmentDeleted, conv->id(), index);
            }
            EMIT(events.draftModified, conv->id());
            continue;
        }

        previouslyUploaded_ += msgSent->size;

        // create DAOs
        auto message = makeMessageDao(dateSent, *msgSent, *draft);
        auto sender = makeSender(message, *draft);

        if (*shouldCancel) throw SyncCanceled();

        SmartSqlite::ScopedTransaction tx(session_, SmartSqlite::Immediate);

        // create message attachments from draft attachments
        std::vector<std::unique_ptr<AttachmentDao>> messageAttachments;
        auto result = AttachmentDao::allForMessage(
                    IsDraft::Yes,
                    draft->conversationId(),
                    session_);
        while (auto attachmentDao = result->next())
        {
            attachmentDao->convertToMessageAttachment(message.id());
            messageAttachments.emplace_back(std::move(attachmentDao));
        }

        // save message
        msgAdder_.addMessage(
                    message, *conv,
                    sender, draft->senderAvatar(),
                    messageAttachments, session_);
        Dao::DeliveryDao(session_).insertDelivery(msgSent->id, delivery);

        tx.commit();

        msgAdder_.emitSignals();
        EMIT(events.messageAttachmentsDownloaded,
             draft->conversationId(),
             msgSent->id);

        clearDraft(*draft, messageAttachments);
    }
}

std::unique_ptr<ConversationDao> MessagesUploader::loadConversation(
        id_type conversationId)
{
    auto conv = ConversationDao::load(conversationId, session_);
    if (!conv)
    {
        throw Db::DatabaseIntegrityError(
                    "MessagesSender::sendNextMessage(): "
                    "couldn't find conversation for draft");
    }
    return conv;
}

void MessagesUploader::clearDraft(
        Dao::DraftDao &draft,
        const std::vector<std::unique_ptr<Dao::AttachmentDao>> &attachments)
{
    for (const auto &att : attachments)
    {
        EMIT(events.draftAttachmentDeleted, draft.conversationId(), att->index());
    }

    draft.clear();
    draft.save();
    EMIT(events.draftModified, draft.conversationId());
}

std::vector<Util::Delivery> MessagesUploader::makeDelivery(const ConversationDao &conv)
{
    std::vector<Util::Delivery> deliveryState;
    for (const auto &part : conv.participantsList())
    {
        Util::Delivery del(Util::KulloAddress(part), Util::Delivery::unsent);
        // create an identifier from hostname and a random string, outside of libkullo
        //del.lockHolder = "myself";
        //del.lockExpires = Util::DateTime::nowUtc() + std::chrono::hours(1);
        deliveryState.emplace_back(del);
    }
    return deliveryState;
}

Protocol::SendableMessage MessagesUploader::makeSendableMessage(
        const Codec::EncodedMessage &encodedMessage)
{
    auto encKeyAndId = privKeyProvider_->getLatestKeyAndId(
                Crypto::AsymmetricKeyType::Encryption);
    auto sigKeyAndId = privKeyProvider_->getLatestKeyAndId(
                Crypto::AsymmetricKeyType::Signature);

    return Codec::MessageEncryptor().makeSendableMessage(
                encodedMessage,
                encKeyAndId.second,
                encKeyAndId.first.pubkey(),
                sigKeyAndId.second,
                sigKeyAndId.first);

}

bool MessagesUploader::ensureSizeLimitCompliance(
        const Protocol::SendableMessage &sendableMsg,
        Dao::DraftDao &draft)
{
    kulloAssert(sendableMsg.keySafe.size() <= MESSAGE_KEY_SAFE_MAX_BYTES);

    return checkAndHandleSizeLimit(
                draft,
                Api::DraftPart::Content,
                sendableMsg.content.size(),
                MESSAGE_CONTENT_MAX_BYTES)
            && checkAndHandleSizeLimit(
                draft,
                Api::DraftPart::Attachments,
                sendableMsg.attachments.size(),
                MESSAGE_ATTACHMENTS_MAX_BYTES);
}

bool MessagesUploader::checkAndHandleSizeLimit(
        Dao::DraftDao &draft, Api::DraftPart part,
        std::size_t size, std::size_t maxSize)
{
    if (size <= maxSize)
    {
        return true;
    }
    else
    {
        Log.w() << "Draft part " << part << " is too large. Size: "
                << Util::Strings::formatReadable(size)
                << " (allowed: "
                << Util::Strings::formatReadable(maxSize)
                << ")";
        resetDraftToEditing(draft);
        EMIT(events.draftModified, draft.conversationId());
        EMIT(events.draftPartTooBig, draft.conversationId(), part, size, maxSize);
        return false;
    }
}

void MessagesUploader::resetDraftToEditing(DraftDao &draft)
{
    draft.setState(DraftState::Editing);
    draft.save();
}

MessageDao MessagesUploader::makeMessageDao(
        const DateTime &dateSent,
        const Protocol::MessageSent &msgSent,
        const DraftDao &draft)
{
    MessageDao message(session_);
    // message data from server
    message.setId(msgSent.id);
    message.setLastModified(msgSent.lastModified);
    message.setDateReceived(msgSent.dateReceived.toString());
    // message data from local data
    message.setConversationId(draft.conversationId());
    message.setSender(settings_.credentials.address->toString());
    message.setDateSent(dateSent.toString());
    message.setText(draft.text());
    message.setFooter(draft.footer());
    message.setState(MessageState::Read, true);
    message.setState(MessageState::Done, true);
    return message;
}

SenderDao MessagesUploader::makeSender(
        const MessageDao &message,
        const DraftDao &draft)
{
    SenderDao sender(*settings_.credentials.address, session_);
    sender.setMessageId(message.id());
    sender.setName(draft.senderName());
    sender.setOrganization(draft.senderOrganization());
    sender.setAvatarHash(Crypto::Hasher::eightByteHash(draft.senderAvatar()));
    sender.setAvatarMimeType(draft.senderAvatarMimeType());
    return sender;
}

}
}
