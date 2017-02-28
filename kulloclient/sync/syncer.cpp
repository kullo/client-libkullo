/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/strings.h"
#include "kulloclient/registry.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

Syncer::Syncer(
        const Db::SessionConfig &sessionConfig,
        const UserSettings &settings,
        const std::shared_ptr<Codec::PrivateKeyProvider> &privKeyProvider) :
    sessionConfig_(sessionConfig),
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

    auto session = Db::makeSession(sessionConfig_);
    kulloAssert(Db::hasCurrentSchema(session));

    auto httpClient = Registry::httpClientFactory()->createHttpClient();

    // sync keys
    progress_.phase = SyncPhase::Keys;
    EMIT(events.progressed, progress_);
    keysSyncer_.reset(new KeysSyncer(settings_.credentials, session, httpClient));
    keysSyncer_->run(shouldCancel);

    // get upload size estimates for progress
    messagesUploader_ = make_unique<MessagesUploader>(
                settings_, privKeyProvider_, session, httpClient);
    progress_.phase = SyncPhase::OutgoingMessages;
    progress_.outgoingMessages = messagesUploader_->initialProgress();
    EMIT(events.progressed, progress_);

    // upload outgoing messages
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
    messagesUploader_->events.draftPartTooBig =
            forwardEvent(events.draftPartTooBig);
    messagesUploader_->events.progressed =
            [this](SyncOutgoingMessagesProgress progress)
    {
        progress_.outgoingMessages = progress;
        EMIT(events.progressed, progress_);
    };

    messagesUploader_->run(shouldCancel);

    // send outgoing messages to their recipients
    messagesSender_ = make_unique<MessagesSender>(
                settings_.credentials, privKeyProvider_, session, httpClient);
    messagesSender_->events.messageModified =
            forwardEvent(events.messageModified);
    messagesSender_->events.progressed =
            [this](SyncOutgoingMessagesProgress progress)
    {
        progress_.outgoingMessages = progress;
        EMIT(events.progressed, progress_);
    };

    messagesSender_->run(shouldCancel, progress_.outgoingMessages.uploadedBytes);

    // sync inbox
    if (mode != SyncMode::SendOnly)
    {
        profileSyncer_ = make_unique<ProfileSyncer>(
                    settings_.credentials, session, httpClient);
        messagesSyncer_ = make_unique<MessagesSyncer>(
                    settings_.credentials, privKeyProvider_, session, httpClient);

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
                [this](SyncIncomingMessagesProgress progress)
        {
            progress_.incomingMessages = progress;
            EMIT(events.progressed, progress_);
        };
        messagesSyncer_->events.finished =
                [this](SyncIncomingMessagesProgress progress)
        {
            progress_.incomingMessages = progress;
        };

        progress_.phase = SyncPhase::Profile;
        EMIT(events.progressed, progress_);
        profileSyncer_->run(shouldCancel);

        progress_.phase = SyncPhase::IncomingMessages;
        EMIT(events.progressed, progress_);
        messagesSyncer_->run(shouldCancel);

        // sync attachments
        if (mode == SyncMode::Everything)
        {
            attachmentSyncer_.reset(
                        new AttachmentSyncer(
                            settings_.credentials, session, httpClient));

            attachmentSyncer_->events.messageAttachmentsDownloaded =
                forwardEvent(events.messageAttachmentsDownloaded);
            attachmentSyncer_->events.progressed =
                    [this](SyncIncomingAttachmentsProgress progress)
            {
                progress_.incomingAttachments = progress;
                EMIT(events.progressed, progress_);
            };

            progress_.phase = SyncPhase::IncomingAttachments;
            EMIT(events.progressed, progress_);
            attachmentSyncer_->run(shouldCancel);
        }
    }

    auto end = std::chrono::steady_clock::now();

    progress_.runTimeMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    Log.i() << "Sync finished successfully. " << progress_;
}

void Syncer::downloadAttachmentsForMessage(
        int64_t msgId, std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    startTime_ = std::chrono::steady_clock::now();

    Db::SharedSessionPtr session = Db::makeSession(sessionConfig_);
    kulloAssert(Db::hasCurrentSchema(session));

    auto httpClient = Registry::httpClientFactory()->createHttpClient();

    attachmentSyncer_.reset(
                new AttachmentSyncer(settings_.credentials, session, httpClient));
    attachmentSyncer_->events.messageAttachmentsDownloaded =
        forwardEvent(events.messageAttachmentsDownloaded);
    attachmentSyncer_->events.progressed =
            [this](SyncIncomingAttachmentsProgress progress)
    {
        progress_.incomingAttachments = progress;
        EMIT(events.progressed, progress_);
    };

    progress_.phase = SyncPhase::IncomingAttachments;
    EMIT(events.progressed, progress_);
    attachmentSyncer_->downloadForMessage(msgId, shouldCancel);

    auto runTime = std::chrono::steady_clock::now() - startTime_;
    progress_.runTimeMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(runTime)
            .count();

    Log.i() << "Sync finished successfully. " << progress_;
}

}
}
