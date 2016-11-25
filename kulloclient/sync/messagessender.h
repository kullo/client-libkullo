/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <tuple>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/codec/codecstructs.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/http/HttpClient.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/protocol/publickeysclient.h"
#include "kulloclient/sync/definitions.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace Sync {

/// Sender for unsent messages
class MessagesSender
{
public:
    struct {
        std::function<void(id_type conversationId, id_type messageId)>
        messageModified;

        /// Emitted when the syncer has made progress.
        std::function<void(SyncOutgoingMessagesProgress progress)>
        progressed;
    } events;

    /**
     * @brief Creates a new message sender, which sends all unsent messages.
     * @param settings Settings for the local user.
     *
     * This doesn't automatically start sending. Call run() to start sending.
     */
    explicit MessagesSender(
            const Util::Credentials &settings,
            const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
            const Db::SharedSessionPtr &session,
            const std::shared_ptr<Http::HttpClient> &httpClient);
    ~MessagesSender();

    /// Start the syncer.
    void run(
            std::shared_ptr<std::atomic<bool>> shouldCancel,
            std::size_t previouslyUploaded);

private:
    void sendMessage(
            id_type currentConversationId,
            id_type currentMessageId,
            const Util::KulloAddress &currentRecipient,
            Protocol::SendableMessage sendableMsg,
            std::size_t estimatedSize);
    void handleNotFound(
            id_type currentConversationId,
            id_type currentMessageId,
            const Util::KulloAddress &currentRecipient);

    Db::SharedSessionPtr session_;
    Protocol::PublicKeysClient keysClient_;
    std::unique_ptr<Protocol::MessagesClient> messagesClient_;
    std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;

    // clang does not yet support std::atomic with int64_t
    std::atomic<size_t> previouslyUploaded_{0};
    std::atomic<size_t> estimatedRemaining_{0};
    SyncOutgoingMessagesProgress progress_;

    K_DISABLE_COPY(MessagesSender);
};

}
}
