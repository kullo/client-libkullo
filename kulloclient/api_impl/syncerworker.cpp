/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/syncerworker.h"

#include <exception>

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
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

SyncerWorker::SyncerWorker(
        std::shared_ptr<std::mutex> syncMutex,
        std::shared_ptr<SessionData> sessionData,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::SyncerRunListener> listener)
    : shouldCancel_(std::make_shared<std::atomic<bool>>(false))
    , syncer_(
          sessionData->dbPath_,
          sessionData->userSettings_->userSettings(),
          sessionData->privKeyProvider_)
    , syncMutex_(syncMutex)
    , sessionData_(sessionData)
    , sessionListener_(sessionListener)
    , listener_(listener)
{
    kulloAssert(syncMutex_);
}

void SyncerWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("sync");
    setupEvents();

    try
    {
        std::unique_lock<std::mutex> lock(*syncMutex_, std::try_to_lock);
        // API clients must not run parallel syncs.
        if (!lock)
        {
            Log.f() << "Tried to run multiple syncs in parallel, which is not "
                       "allowed. Terminating.";
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

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(toNetworkError(std::current_exception()));
        }
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->finished();
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

    syncer_.events.draftAttachmentsTooBig =
            [this](
            id_type conversationId,
            std::size_t size,
            std::size_t sizeAllowed)
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            K_UNUSED(size);
            K_UNUSED(sizeAllowed);
            listener_->draftAttachmentsTooBig(conversationId);
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
