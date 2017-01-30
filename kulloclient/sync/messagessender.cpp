/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/messagessender.h"

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/codec/messagecompressor.h"
#include "kulloclient/codec/messageencryptor.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/daoutil.h"
#include "kulloclient/dao/deliverydao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/http/TransferProgress.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/numeric_cast.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

MessagesSender::MessagesSender(
        const Credentials &credentials,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
        const Db::SharedSessionPtr &session,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : session_(session)
    , keysClient_(httpClient)
    , privKeyProvider_(privKeyProvider)
{
    messagesClient_.reset(new Protocol::MessagesClient(
                             *credentials.address,
                             *credentials.masterKey,
                              httpClient));
}

MessagesSender::~MessagesSender()
{
}

void MessagesSender::run(
        std::shared_ptr<std::atomic<bool>> shouldCancel,
        std::size_t previouslyUploaded)
{
    previouslyUploaded_ = previouslyUploaded;

    id_type currentConversationId = 0;
    id_type currentMessageId = 0;
    std::size_t currentMessageEstimatedSize = 0;
    Codec::EncodedMessage encodedMessage;
    Crypto::AsymmetricKeyLoader loader;
    Codec::MessageEncryptor encryptor;

    auto idsAndRecipients = Dao::DeliveryDao(session_).unsentMessages();
    for (const auto &idAndRecipient : idsAndRecipients)
    {
        if (*shouldCancel) throw SyncCanceled();

        estimatedRemaining_ = Dao::MessageDao::sizeOfAllUndelivered(session_);
        progress_.uploadedBytes = previouslyUploaded_;
        progress_.totalBytes = previouslyUploaded_ + estimatedRemaining_;
        EMIT(events.progressed, progress_);

        auto oldMessageId = currentMessageId;
        currentMessageId = std::get<0>(idAndRecipient);
        auto currentRecipient = std::get<1>(idAndRecipient);

        if (oldMessageId != currentMessageId)
        {
            // load message
            auto messageDao = Dao::MessageDao::load(
                        currentMessageId, Dao::Old::No, session_);
            if (!messageDao || messageDao->deleted())
            {
                Dao::DeliveryDao(session_).remove(currentMessageId);
                continue;
            }
            currentConversationId = messageDao->conversationId();

            if (*shouldCancel) throw SyncCanceled();

            // encode message
            encodedMessage = Codec::MessageEncoder::encodeMessage(
                        *messageDao, session_);
            // must be calculated the same way as in sizeOfAllUndelivered()
            currentMessageEstimatedSize =
                    Dao::PER_MESSAGE_OVERHEAD +
                    messageDao->text().size() +
                    encodedMessage.attachments.size();
            Codec::MessageCompressor().compress(encodedMessage);
        }

        if (*shouldCancel) throw SyncCanceled();

        try
        {
            auto encKeyPair = keysClient_.getPublicKey(
                        currentRecipient,
                        Protocol::PublicKeysClient::LATEST_ENCRYPTION_PUBKEY);

            if (*shouldCancel) throw SyncCanceled();

            auto sigKeyAndId = privKeyProvider_->getLatestKeyAndId(
                        Crypto::AsymmetricKeyType::Signature);

            if (*shouldCancel) throw SyncCanceled();

            auto encKey = loader.loadPublicKey(
                        Crypto::AsymmetricKeyType::Encryption,
                        encKeyPair.pubkey);

            if (*shouldCancel) throw SyncCanceled();

            auto sendableMsg = encryptor.makeSendableMessage(
                        encodedMessage,
                        encKeyPair.id,
                        encKey,
                        sigKeyAndId.second,
                        sigKeyAndId.first);

            if (*shouldCancel) throw SyncCanceled();

            sendMessage(shouldCancel,
                        currentConversationId,
                        currentMessageId,
                        currentRecipient,
                        sendableMsg,
                        currentMessageEstimatedSize);
        }
        catch (Protocol::Canceled&)
        {
            throw SyncCanceled();
        }
        catch (Protocol::NotFound)
        {
            handleNotFound(
                        currentConversationId,
                        currentMessageId,
                        currentRecipient);
            continue;
        }
    }
}

void MessagesSender::sendMessage(
        std::shared_ptr<std::atomic<bool>> shouldCancel,
        id_type currentConversationId,
        id_type currentMessageId,
        const KulloAddress &currentRecipient,
        Protocol::SendableMessage sendableMsg,
        std::size_t estimatedSize)
{
    try
    {
        previouslyUploaded_ += messagesClient_->sendMessage(
                    currentRecipient,
                    sendableMsg,
                    [this, estimatedSize, shouldCancel]
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
                        Util::numeric_cast<int64_t>(estimatedSize) +
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
    catch (Protocol::NotFound&)
    {
        handleNotFound(
                    currentConversationId,
                    currentMessageId,
                    currentRecipient);
        return;
    }

    Dao::DeliveryDao(session_).markAsDelivered(
                currentMessageId,
                currentRecipient,
                Util::DateTime::nowUtc());
    EMIT(events.messageModified, currentConversationId, currentMessageId);
}

void MessagesSender::handleNotFound(
        id_type currentConversationId,
        id_type currentMessageId,
        const KulloAddress &currentRecipient)
{
    Dao::DeliveryDao(session_).markAsFailed(
                currentMessageId,
                currentRecipient,
                Util::DateTime::nowUtc(),
                Util::Delivery::doesnt_exist);
    EMIT(events.messageModified, currentConversationId, currentMessageId);
}

}
}
