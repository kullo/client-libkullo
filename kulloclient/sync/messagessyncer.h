/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <boost/optional.hpp>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/codec/codecstructs.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/protocol/publickeysclient.h"
#include "kulloclient/sync/definitions.h"
#include "kulloclient/sync/messageadder.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/usersettings.h"

namespace SmartSqlite {
class ScopedTransaction;
}

namespace Kullo {
namespace Sync {

/**
 * @brief Syncer for messages
 */
class MessagesSyncer
{
public:
    struct {
        /**
         * @brief Emitted when a participant was added or modified by the syncer.
         * @param address The participant's address
         */
        std::function<void(std::string address)>
        participantAddedOrModified;

        /**
         * @brief Emitted when a new message has been added by the syncer.
         * @param conversationId The message's conversation ID
         * @param id The message's ID
         */
        std::function<void(Kullo::id_type conversationId, Kullo::id_type id)>
        messageAdded;

        /**
         * @brief Emitted when a message has been modified by the syncer.
         * @param conversationId The message's conversation ID
         * @param id The message's ID
         */
        std::function<void(Kullo::id_type conversationId, Kullo::id_type id)>
        messageModified;

        /**
         * @brief Emitted when a message has been deleted by on the server.
         * @param conversationId The message's conversation ID
         * @param id The message's ID
         */
        std::function<void(Kullo::id_type conversationId, Kullo::id_type id)>
        messageDeleted;

        std::function<void(id_type conversationId, id_type messageId)>
        senderAdded;

        /**
         * @brief Emitted when a conversation has been added by the syncer.
         * @param conversationId The new conversation's ID
         */
        std::function<void(Kullo::id_type conversationId)>
        conversationAdded;

        /**
         * @brief Emitted when a conversation has been modified by the syncer.
         *
         * E.g when a message is added or deleted.
         * @param conversationId The message's conversation ID
         */
        std::function<void(Kullo::id_type conversationId)>
        conversationModified;

        /// Emitted when the syncer has made progress.
        std::function<void(SyncMessagesProgress progress)>
        progressed;

        /// Emitted when the syncer has finished successfully.
        std::function<void(SyncMessagesProgress progress)>
        finished;
    } events;

    /**
     * @brief Creates a new message syncer.
     * @param username Username of the local user (e.g. john.doe#kullo.net)
     *
     * This doesn't automatically start the syncer. Call start() to start syncing.
     */
    explicit MessagesSyncer(
            const Util::UserSettings &settings,
            std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider,
            Db::SharedSessionPtr session);

    ~MessagesSyncer();

    /**
     * @brief Start the syncer.
     */
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    // downloadAndProcessMessages and friends
    void downloadAndProcessMessages();
    void handleNewTombstone(const Kullo::Protocol::Message &httpMsg);
    void handleNewMessage(const Kullo::Protocol::Message &httpMsg);
    void handleDeletedMessage(
            const Protocol::Message &httpMsg,
            Dao::MessageDao &local);
    void handleModifiedMessage(
            const Kullo::Protocol::Message &httpMsg,
            Dao::MessageDao &local);
    void downloadPublicKey(const Util::KulloAddress &address, id_type sigKeyId);

    // uploadModifications and friends
    void uploadModifications();
    void handleSuccessfulModification(const Protocol::IdLastModified &idlm);

    // general utils
    void yieldIfNecessary();
    Crypto::SymmetricKey getPrivateDataKey();
    std::unique_ptr<Codec::MessageDecoder> makeDecoder(
            const Codec::DecryptedMessage &decryptedMessage);
    boost::optional<Codec::DecryptedMessage> tryDecrypt(
            const Protocol::Message &httpMsg);
    std::unique_ptr<Codec::MessageDecoder> tryDecode(
            const Codec::DecryptedMessage &decryptedMessage,
            SmartSqlite::ScopedTransaction &tx);

    std::shared_ptr<std::atomic<bool>> shouldCancel_;
    std::unique_ptr<Protocol::MessagesClient> client_;
    Protocol::PublicKeysClient pubKeysClient_;
    Db::SharedSessionPtr session_;
    Util::UserSettings settings_;

    MessageAdder msgAdder_;
    std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;
    std::unique_ptr<Crypto::SymmetricKey> privateDataKey_;

    SyncMessagesProgress progress_;

    using clock = std::chrono::steady_clock;
    clock::time_point lastYield_;

    K_DISABLE_COPY(MessagesSyncer);
};

}
}
