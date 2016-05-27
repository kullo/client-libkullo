/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/syncer.h"

#include <chrono>

#include "kulloclient/sync/attachmentsyncer.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/sync/keyssyncer.h"
#include "kulloclient/sync/messagessender.h"
#include "kulloclient/sync/messagessyncer.h"
#include "kulloclient/sync/messagesuploader.h"
#include "kulloclient/sync/profilesyncer.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/events.h"
#include "kulloclient/util/formatstring.h"
#include "kulloclient/util/librarylogger.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

Syncer::Syncer(
        const std::string &dbFile,
        const UserSettings &settings,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider) :
    dbFile_(dbFile),
    settings_(settings),
    privKeyProvider_(privKeyProvider)
{}

Syncer::~Syncer()
{}

void Syncer::run(
        SyncMode mode,
        std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    auto start = std::chrono::steady_clock::now();

    Db::SharedSessionPtr session = Db::makeSession(dbFile_);
    kulloAssert(Db::hasCurrentSchema(session));

    // sync keys
    keysSyncer_.reset(new KeysSyncer(settings_.credentials, session));
    keysSyncer_->run(shouldCancel);

    // sync outgoing messages
    messagesUploader_ = make_unique<MessagesUploader>(
                settings_, privKeyProvider_, session);
    messagesSender_ = make_unique<MessagesSender>(
                settings_.credentials, privKeyProvider_, session);

    messagesUploader_->events.conversationAdded =
            forwardEvent(events.conversationAdded);
    messagesUploader_->events.conversationModified =
            forwardEvent(events.conversationModified);
    messagesUploader_->events.messageAdded =
            forwardEvent(events.messageAdded);
    messagesUploader_->events.senderAdded =
            forwardEvent(events.senderAdded);
    messagesUploader_->events.messageAttachmentsDownloaded =
            forwardEvent(events.messageAttachmentsDownloaded);
    messagesUploader_->events.draftModified =
            forwardEvent(events.draftModified);
    messagesUploader_->events.draftAttachmentDeleted =
            forwardEvent(events.draftAttachmentDeleted);
    messagesUploader_->events.draftAttachmentsTooBig =
            forwardEvent(events.draftAttachmentsTooBig);
    messagesSender_->events.messageModified =
            forwardEvent(events.messageModified);

    messagesUploader_->run(shouldCancel);
    messagesSender_->run(shouldCancel);

    // sync inbox
    if (mode != SyncMode::SendOnly)
    {
        profileSyncer_ = make_unique<ProfileSyncer>(
                    settings_.credentials, session);
        messagesSyncer_ = make_unique<MessagesSyncer>(
                    settings_.credentials, privKeyProvider_, session);

        profileSyncer_->events.profileModified =
            forwardEvent(events.profileModified);
        messagesSyncer_->events.messageAdded =
            forwardEvent(events.messageAdded);
        messagesSyncer_->events.messageModified =
            forwardEvent(events.messageModified);
        messagesSyncer_->events.messageDeleted =
            forwardEvent(events.messageDeleted);
        messagesSyncer_->events.senderAdded =
            forwardEvent(events.senderAdded);
        messagesSyncer_->events.conversationAdded =
            forwardEvent(events.conversationAdded);
        messagesSyncer_->events.conversationModified =
            forwardEvent(events.conversationModified);
        messagesSyncer_->events.progressed =
                [this](SyncMessagesProgress progress)
        {
            progress_.messages = progress;
            Log.d() << "Sync progress: " << progress_;
            EMIT(events.progressed, progress_);
        };
        messagesSyncer_->events.finished =
                [this](SyncMessagesProgress progress)
        {
            progress_.messages = progress;
        };

        profileSyncer_->run(shouldCancel);
        messagesSyncer_->run(shouldCancel);

        // sync attachments
        if (mode == SyncMode::Everything)
        {
            attachmentSyncer_.reset(
                        new AttachmentSyncer(settings_.credentials, session));

            attachmentSyncer_->events.messageAttachmentsDownloaded =
                forwardEvent(events.messageAttachmentsDownloaded);

            attachmentSyncer_->run(shouldCancel);
        }
    }

    auto end = std::chrono::steady_clock::now();

    progress_.runTimeMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    Log.i() << "Sync finished successfully. " << progress_;
    EMIT(events.finished, progress_);
}

void Syncer::downloadAttachmentsForMessage(
        int64_t msgId, std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    startTime_ = std::chrono::steady_clock::now();

    Db::SharedSessionPtr session = Db::makeSession(dbFile_);
    kulloAssert(Db::hasCurrentSchema(session));

    attachmentSyncer_.reset(
                new AttachmentSyncer(settings_.credentials, session));
    attachmentSyncer_->events.messageAttachmentsDownloaded =
        forwardEvent(events.messageAttachmentsDownloaded);
    attachmentSyncer_->downloadForMessage(msgId, shouldCancel);

    auto runTime = std::chrono::steady_clock::now() - startTime_;
    progress_.runTimeMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(runTime)
            .count();

    EMIT(events.finished, progress_);
}

}
}
