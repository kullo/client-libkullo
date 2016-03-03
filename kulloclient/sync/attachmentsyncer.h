/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Sync {

/**
 * @brief Syncer for message attachments
 */
class AttachmentSyncer
{
public:
    struct
    {
        /**
         * @brief Called when all attachments of a message have been downloaded.
         * @param conversationId The message's conversation ID
         * @param messageId The message's ID
         */
        std::function<void(id_type conversationId, id_type messageId)>
        messageAttachmentsDownloaded;
    } events;

    /**
     * @brief Creates a new attachment syncer.
     * @param username Username of the local user (e.g. john.doe#kullo.net)
     *
     * This doesn't automatically start the syncer. Call start() to start syncing.
     */
    explicit AttachmentSyncer(
            const Util::UserSettings &settings,
            Db::SharedSessionPtr session);
    ~AttachmentSyncer();

    /// @brief Start the syncer.
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);

    void downloadForMessage(
            int64_t msgId, std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    Db::SharedSessionPtr session_;
    std::unique_ptr<Protocol::MessagesClient> client_;

    K_DISABLE_COPY(AttachmentSyncer);
};

}
}
