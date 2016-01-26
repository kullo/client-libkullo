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
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/protocol/publickeysclient.h"
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
    } events;

    /**
     * @brief Creates a new message sender, which sends all unsent messages.
     * @param settings Settings for the local user.
     *
     * This doesn't automatically start sending. Call run() to start sending.
     */
    explicit MessagesSender(
            const Util::UserSettings &settings,
            std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider,
            Db::SharedSessionPtr session);
    ~MessagesSender();

    /// Start the syncer.
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    void sendMessage(
            id_type currentConversationId,
            id_type currentMessageId,
            const Util::KulloAddress &currentRecipient,
            Protocol::SendableMessage sendableMsg);
    void handleNotFound(
            id_type currentConversationId,
            id_type currentMessageId,
            const Util::KulloAddress &currentRecipient);

    Db::SharedSessionPtr session_;
    Util::UserSettings settings_;
    Protocol::PublicKeysClient keysClient_;
    std::unique_ptr<Protocol::MessagesClient> messagesClient_;
    std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;

    K_DISABLE_COPY(MessagesSender);
};

}
}
