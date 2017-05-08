/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/api/DraftPart.h"
#include "kulloclient/codec/codecstructs.h"
#include "kulloclient/dao/senderdao.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/sync/definitions.h"
#include "kulloclient/sync/messageadder.h"
#include "kulloclient/util/delivery.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace Sync {

/**
 * @brief Converts a draft to a message in the sender's inbox
 *
 * When a draft has been converted, it is cleared but not deleted.
 */
class MessagesUploader
{
public:
    struct {
        std::function<void(id_type conversationId)>
        conversationAdded;

        std::function<void(id_type conversationId)>
        conversationModified;

        std::function<void(id_type conversationId, id_type messageId)>
        messageAdded;

        std::function<void(id_type conversationId, id_type messageId)>
        senderAdded;

        std::function<void(id_type conversationId, id_type messageId)>
        messageAttachmentsDownloaded;

        /**
         * @brief Emitted when a draft has been modified by the syncer.
         * @param conversationId The draft's conversation ID
         */
        std::function<void(id_type conversationId)>
        draftModified;

        std::function<void(id_type conversationId, id_type attachmentId)>
        draftAttachmentDeleted;

        std::function<void(
                id_type conversationId,
                Api::DraftPart part,
                std::size_t currentSize,
                std::size_t maxSize)>
        draftPartTooBig;

        /// Emitted when the syncer has made progress.
        std::function<void(SyncOutgoingMessagesProgress progress)>
        progressed;
    } events;

    /**
     * @brief Creates a new message uploader.
     *
     * @param settings Settings for the local user.
     * @param session DB session to be used
     *
     * This doesn't automatically start uploading, see run().
     */
    explicit MessagesUploader(
            const Util::UserSettings &settings,
            const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider,
            const Db::SharedSessionPtr &session,
            const std::shared_ptr<Http::HttpClient> &httpClient);
    ~MessagesUploader();

    SyncOutgoingMessagesProgress initialProgress();

    /// Start the syncer.
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    std::unique_ptr<Dao::ConversationDao> loadConversation(
            id_type conversationId);
    void clearDraft(
            Dao::DraftDao &draft,
            const std::vector<std::unique_ptr<Dao::AttachmentDao>> &attachments);
    std::vector<Util::Delivery> makeDelivery(const Dao::ConversationDao &conv);
    Protocol::SendableMessage makeSendableMessage(
            const Codec::EncodedMessage &encodedMessage);
    bool ensureSizeLimitCompliance(
            const Protocol::SendableMessage &sendableMsg,
            Dao::DraftDao &draft);
    bool checkAndHandleSizeLimit(
            Dao::DraftDao &draft, Api::DraftPart part,
            std::size_t size, std::size_t maxSize);
    Dao::MessageDao makeMessageDao(
            const Util::DateTime &dateSent,
            const Protocol::MessageSent &msgSent,
            const Dao::DraftDao &draft);
    Dao::SenderDao makeSender(
            const Dao::MessageDao &message,
            const Dao::DraftDao &draft);

    Db::SharedSessionPtr session_;
    Util::UserSettings settings_;
    std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider_;
    std::unique_ptr<Protocol::MessagesClient> messagesClient_;
    MessageAdder msgAdder_;

    // clang does not yet support std::atomic with int64_t
    std::atomic<size_t> previouslyUploaded_{0};
    std::atomic<size_t> estimatedRemaining_{0};
    SyncOutgoingMessagesProgress progress_;

    K_DISABLE_COPY(MessagesUploader);
};

}
}
