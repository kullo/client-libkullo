/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/messagessender.h"

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/codec/messagecompressor.h"
#include "kulloclient/codec/messageencryptor.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/dao/conversationdao.h"
#include "kulloclient/dao/deliverydao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/protocol/messagesclient.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

MessagesSender::MessagesSender(
        const Credentials &credentials,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
        const Db::SharedSessionPtr &session)
    : session_(session),
      privKeyProvider_(privKeyProvider)
{
    messagesClient_.reset(new Protocol::MessagesClient(
                             *credentials.address,
                             *credentials.masterKey));
}

MessagesSender::~MessagesSender()
{
}

void MessagesSender::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    id_type currentConversationId = 0;
    id_type currentMessageId = 0;
    Codec::EncodedMessage encodedMessage;

    auto idsAndRecipients = Dao::DeliveryDao(session_).unsentMessages();
    for (const auto &idAndRecipient : idsAndRecipients)
    {
        if (*shouldCancel) throw SyncCanceled();

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

            // encode message
            encodedMessage = Codec::MessageEncoder::encodeMessage(
                        *messageDao, session_);
            Codec::MessageCompressor().compress(encodedMessage);
        }

        Protocol::KeyPair encKeyPair;
        try
        {
            encKeyPair = keysClient_.getPublicKey(
                        currentRecipient,
                        Protocol::PublicKeysClient::LATEST_ENCRYPTION_PUBKEY);
        }
        catch (Protocol::NotFound)
        {
            handleNotFound(
                        currentConversationId,
                        currentMessageId,
                        currentRecipient);
            continue;
        }

        auto sigKeyAndId = privKeyProvider_->getLatestKeyAndId(
                    Crypto::AsymmetricKeyType::Signature);

        Crypto::AsymmetricKeyLoader loader;
        auto encKey = loader.loadPublicKey(
                    Crypto::AsymmetricKeyType::Encryption,
                    encKeyPair.pubkey);

        Codec::MessageEncryptor encryptor;
        auto sendableMsg = encryptor.makeSendableMessage(
                    encodedMessage,
                    encKeyPair.id,
                    encKey,
                    sigKeyAndId.second,
                    sigKeyAndId.first);

        sendMessage(currentConversationId,
                    currentMessageId,
                    currentRecipient,
                    sendableMsg);
    }
}

void MessagesSender::sendMessage(
        id_type currentConversationId,
        id_type currentMessageId,
        const KulloAddress &currentRecipient,
        Protocol::SendableMessage sendableMsg)
{
    try
    {
        messagesClient_->sendMessage(currentRecipient, sendableMsg);
    }
    catch (Protocol::NotFound)
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
