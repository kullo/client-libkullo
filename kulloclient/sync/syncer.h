/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/types.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/sync/definitions.h"
#include "kulloclient/util/usersettings.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Sync {

/**
 * @brief Syncer that coordinates all other syncers.
 */
class Syncer
{
public:
    struct {
        ///  Emitted when the syncer has made progress.
        std::function<void(SyncProgress progress)>
        progressed;

        ///  Emitted when the syncer has finished successfully.
        std::function<void(SyncProgress progress)>
        finished;

        std::function<void(std::string address)>
        participantAddedOrModified;

        std::function<void(id_type conversationId)>
        conversationAdded;

        std::function<void(id_type conversationId)>
        conversationModified;

        /**
         * @brief Emitted when a draft has been modified by the syncer.
         * @param conversationId The draft's conversation ID
         */
        std::function<void(id_type conversationId)>
        draftModified;

        std::function<void(id_type conversationId, id_type messageId)>
        messageAdded;

        std::function<void(id_type conversationId, id_type messageId)>
        messageModified;

        std::function<void(id_type conversationId, id_type messageId)>
        messageDeleted;

        std::function<void(id_type conversationId, id_type messageId)>
        senderAdded;

        std::function<void(id_type conversationId, id_type messageId)>
        messageAttachmentsDownloaded;

        std::function<void(id_type conversationId, id_type attachmentId)>
        draftAttachmentDeleted;

        std::function<void(
                id_type conversationId,
                std::size_t size,
                std::size_t sizeAllowed)>
        draftAttachmentsTooBig;
    } events;

    /**
     * @brief Creates new syncer.
     * @param dbFile The SQLite DB file that should be used.
     * @param settings Settings for the local user.
     */
    Syncer(
            const std::string &dbFile,
            const Util::UserSettings &settings,
            const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider);
    ~Syncer();

    void run(
            SyncMode mode,
            std::shared_ptr<std::atomic<bool>> shouldCancel);

    void downloadAttachmentsForMessage(
            int64_t msgId,
            std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    std::chrono::steady_clock::time_point startTime_;
    std::string dbFile_;
    Util::UserSettings settings_;
    std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;
    std::unique_ptr<KeysSyncer> keysSyncer_;
    std::unique_ptr<MessagesUploader> messagesUploader_;
    std::unique_ptr<MessagesSender> messagesSender_;
    std::unique_ptr<MessagesSyncer> messagesSyncer_;
    std::unique_ptr<AttachmentSyncer> attachmentSyncer_;
    SyncProgress progress_;

    K_DISABLE_COPY(Syncer);
};

}
}
