/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/sync/definitions.h"
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

        /// Emitted when the syncer has made progress.
        std::function<void(SyncIncomingAttachmentsProgress progress)>
        progressed;
    } events;

    /**
     * @brief Creates a new attachment syncer.
     * @param username Username of the local user (e.g. john.doe#kullo.net)
     *
     * This doesn't automatically start the syncer. Call run() to start syncing.
     */
    explicit AttachmentSyncer(
            const Util::Credentials &settings,
            const Db::SharedSessionPtr &session,
            const std::shared_ptr<Http::HttpClient> &httpClient);
    ~AttachmentSyncer();

    /// @brief Start the syncer.
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);

    size_t downloadForMessage(
            int64_t msgId, std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    Db::SharedSessionPtr session_;
    std::unique_ptr<Protocol::MessagesClient> messagesClient_;
    std::atomic<size_t> previouslyDownloaded_ {0};
    std::atomic<size_t> estimatedRemaining_ {0};

    SyncIncomingAttachmentsProgress progress_;

    K_DISABLE_COPY(AttachmentSyncer);
};

}
}
