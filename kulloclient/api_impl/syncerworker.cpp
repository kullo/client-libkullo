/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/syncerworker.h"

#include <exception>

#include "kulloclient/api/SyncProgress.h"
#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/event/conversationaddedevent.h"
#include "kulloclient/event/conversationmodifiedevent.h"
#include "kulloclient/event/draftattachmentremovedevent.h"
#include "kulloclient/event/draftmodifiedevent.h"
#include "kulloclient/event/messageaddedevent.h"
#include "kulloclient/event/messageattachmentsdownloadedevent.h"
#include "kulloclient/event/messagemodifiedevent.h"
#include "kulloclient/event/messageremovedevent.h"
#include "kulloclient/event/senderaddedevent.h"
#include "kulloclient/event/usersettingmodifiedevent.h"
#include "kulloclient/sync/definitions.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

SyncerWorker::SyncerWorker(
        std::shared_ptr<SessionData> sessionData,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::SyncerListener> listener)
    : shouldCancel_(std::make_shared<std::atomic<bool>>(false))
    , syncer_(
          sessionData->sessionConfig_,
          sessionData->userSettings_->userSettings(),
          sessionData->privKeyProvider_)
    , sessionListener_(sessionListener)
    , listener_(listener)
{}

void SyncerWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("sync");
    setupEvents();

    try
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (auto listener = listener_)
        {
            lock.unlock();
            listener->started();
        }
        doWork();
    }
    catch (Sync::SyncCanceled &)
    {
        // just quit on canceled sync
        return;
    }
    catch (std::exception &ex)
    {
        Log.e() << "SyncerWorker failed: " << Util::formatException(ex);

        std::unique_lock<std::mutex> lock(mutex_);
        if (auto listener = listener_)
        {
            lock.unlock();
            listener->error(toNetworkError(std::current_exception()));
        }
        return;
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (auto listener = listener_)
        {
            lock.unlock();
            listener->finished();
        }
    }

    Log.d() << "Done syncing and sending events.";
}

void SyncerWorker::cancel()
{
    *shouldCancel_ = true;

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    sessionListener_.reset();
    listener_.reset();
}

void SyncerWorker::setupEvents()
{
    syncer_.events.conversationAdded =
            [this](id_type conversationId)
    {
        sendEvent(std::make_shared<Event::ConversationAddedEvent>(
                      conversationId));
    };

    syncer_.events.conversationModified =
            [this](id_type conversationId)
    {
        sendEvent(std::make_shared<Event::ConversationModifiedEvent>(
                      conversationId));
    };

    syncer_.events.draftModified =
            [this](id_type conversationId)
    {
        sendEvent(std::make_shared<Event::DraftModifiedEvent>(conversationId));
    };

    syncer_.events.messageAdded =
            [this](id_type conversationId, id_type messageId)
    {
        sendEvent(std::make_shared<Event::MessageAddedEvent>(conversationId, messageId));
    };

    syncer_.events.messageModified =
            [this](id_type conversationId, id_type messageId)
    {
        sendEvent(std::make_shared<Event::MessageModifiedEvent>(conversationId, messageId));
    };

    syncer_.events.messageDeleted =
            [this](id_type conversationId, id_type messageId)
    {
        sendEvent(std::make_shared<Event::MessageRemovedEvent>(conversationId, messageId));
    };

    syncer_.events.senderAdded =
            [this](id_type conversationId, id_type messageId)
    {
        K_UNUSED(conversationId);
        sendEvent(std::make_shared<Event::SenderAddedEvent>(messageId));
    };

    syncer_.events.messageAttachmentsDownloaded =
            [this](id_type conversationId, id_type messageId)
    {
        K_UNUSED(conversationId);
        sendEvent(std::make_shared<Event::MessageAttachmentsDownloadedEvent>(
                      messageId));
    };

    syncer_.events.draftAttachmentDeleted =
            [this](id_type conversationId, id_type attachmentId)
    {
        sendEvent(std::make_shared<Event::DraftAttachmentRemovedEvent>(
                      conversationId, attachmentId));
    };

    syncer_.events.draftPartTooBig =
            [this](
            id_type conversationId,
            Api::DraftPart part,
            std::size_t size,
            std::size_t maxSize)
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            K_UNUSED(size);
            K_UNUSED(maxSize);
            listener_->draftPartTooBig(conversationId, part, size, maxSize);
        }
    };

    syncer_.events.profileModified =
            [this](const std::string &key)
    {
        sendEvent(std::make_shared<Event::UserSettingModifiedEvent>(key));
    };

    syncer_.events.progressed =
            [this](Sync::SyncProgress progress)
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            Log.d() << "Sync progress: " << progress;
            listener_->progressed(Api::SyncProgress(
                                      implToApi(progress.phase),
                                      progress.incomingMessages.processedMessages,
                                      progress.incomingMessages.totalMessages,
                                      progress.incomingMessages.newMessages,
                                      progress.incomingMessages.newUnreadMessages,
                                      progress.incomingMessages.modifiedMessages,
                                      progress.incomingMessages.deletedMessages,
                                      progress.incomingAttachments.downloadedBytes,
                                      progress.incomingAttachments.totalBytes,
                                      progress.outgoingMessages.uploadedBytes,
                                      progress.outgoingMessages.totalBytes,
                                      progress.runTimeMs
                                      ));
        }
    };
}

void SyncerWorker::sendEvent(
        const std::shared_ptr<Api::InternalEvent> &event)
{
    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    if (sessionListener_)
    {
        sessionListener_->internalEvent(event);
    }
}

}
}
