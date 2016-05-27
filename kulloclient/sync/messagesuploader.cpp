/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/messagesuploader.h"

#include <functional>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/codec/messagecompressor.h"
#include "kulloclient/codec/messageencryptor.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/hasher.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/deliverydao.h"
#include "kulloclient/dao/symmetrickeydao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/formatstring.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Dao;
using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

MessagesUploader::MessagesUploader(
        const UserSettings &settings,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
        const Db::SharedSessionPtr &session)
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
                             *settings_.credentials.masterKey));
}

MessagesUploader::~MessagesUploader()
{
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

        auto conv = loadConversation(draft->conversationId());

        // encode message
        auto dateSent = DateTime::nowUtc();
        auto encodedMessage = Codec::MessageEncoder::encodeMessage(
                    settings_.credentials,
                    *draft,
                    dateSent,
                    session_);
        Codec::MessageCompressor().compress(encodedMessage);
        auto sendableMsg = makeSendableMessage(encodedMessage);
        if (!ensureSizeLimitCompliance(sendableMsg, *draft)) continue;

        // create and encode meta
        auto delivery = makeDelivery(*conv);
        auto encodedMeta = Codec::MessageEncoder::encodeMeta(
                    true,  // read
                    true,  // done
                    delivery);
        auto meta = Codec::MessageEncryptor().encryptMeta(
                    encodedMeta,
                    Dao::SymmetricKeyDao::loadKey(
                        Dao::SymmetricKeyDao::PRIVATE_DATA_KEY,
                        session_));

        // send message
        auto msgSent = messagesClient_->sendMessageToSelf(sendableMsg, meta);

        // create DAOs
        auto message = makeMessageDao(dateSent, msgSent, *draft);
        auto sender = makeSender(message, *draft);

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
        Dao::DeliveryDao(session_).insertDelivery(msgSent.id, delivery);

        tx.commit();

        msgAdder_.emitSignals();
        EMIT(events.messageAttachmentsDownloaded,
             draft->conversationId(),
             msgSent.id);

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

    auto attachmentsSize = sendableMsg.attachments.size();
    if (attachmentsSize > MESSAGE_ATTACHMENTS_SEND_MAX_BYTES)
    {
        Log.w() << "Attachments are too large. Size: "
                << Util::FormatString::formatIntegerWithCommas(attachmentsSize)
                << " (allowed: "
                << Util::FormatString::formatIntegerWithCommas(MESSAGE_ATTACHMENTS_SEND_MAX_BYTES)
                << ")";
        draft.setState(DraftState::Editing);
        draft.save();
        EMIT(events.draftAttachmentsTooBig,
             draft.conversationId(),
             attachmentsSize,
             MESSAGE_ATTACHMENTS_SEND_MAX_BYTES);
        EMIT(events.draftModified, draft.conversationId());
        return false;
    }
    else
    {
        return true;
    }
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

ParticipantDao MessagesUploader::makeSender(
        const MessageDao &message,
        const DraftDao &draft)
{
    ParticipantDao sender(*settings_.credentials.address, session_);
    sender.setMessageId(message.id());
    sender.setName(draft.senderName());
    sender.setOrganization(draft.senderOrganization());
    sender.setAvatarHash(Crypto::Hasher::eightByteHash(draft.senderAvatar()));
    sender.setAvatarMimeType(draft.senderAvatarMimeType());
    return sender;
}

}
}
