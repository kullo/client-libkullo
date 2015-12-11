/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/syncerimpl.h"

#include <chrono>
#include <thread>

#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/syncerdownloadattachmentsformessageworker.h"
#include "kulloclient/api_impl/syncerrunworker.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

SyncerImpl::SyncerImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
    , task_(std::make_shared<SyncerAsyncTask>(*this))
    , taskSyncerListener_(std::make_shared<SyncerSyncerListener>(*this))
    , running_(false)
{
    kulloAssert(sessionData_);
}

void SyncerImpl::setListener(
        const std::shared_ptr<Api::SyncerListener> &listener)
{
    listener_ = listener;
}

void SyncerImpl::requestSync(Api::SyncMode mode)
{
    std::lock_guard<std::recursive_mutex> lock(queueMutex_); K_RAII(lock);

    switch (mode)
    {
    case Api::SyncMode::SendOnly:
        // "SendOnly" is included by all other syncs, so only enqueue it if
        // there is no other queued sync.
        if (!enqueuedSync_)
        {
            enqueuedSync_ = mode;
        }
        break;

    case Api::SyncMode::WithoutAttachments:
        // "WithoutAttachments" is only enclosed by "Everything" but overwrites
        // "SendOnly".
        if (enqueuedSync_ != Api::SyncMode::Everything)
        {
            enqueuedSync_ = mode;
        }
        break;

    case Api::SyncMode::Everything:
        // "Everything" does not only enclose the other sync modes but also all
        // message attachment download requests.
        enqueuedSync_ = mode;
        messageAttachmentDownloadQueue_.clear();
        break;

    default:
        kulloAssert(false);
    }

    runNextJobIfIdle();
}

void SyncerImpl::requestDownloadingAttachmentsForMessage(int64_t msgId)
{
    kulloAssert(msgId >= ID_MIN && msgId <= ID_MAX);

    std::lock_guard<std::recursive_mutex> lock(queueMutex_); K_RAII(lock);

    // "Everything" sync encloses all attachments, so exclude that
    if (enqueuedSync_ != Api::SyncMode::Everything)
    {
        // append job only if msgId is not already enqueued
        auto begin = messageAttachmentDownloadQueue_.begin();
        auto end = messageAttachmentDownloadQueue_.end();
        if (std::find(begin, end, msgId) == end)
        {
            messageAttachmentDownloadQueue_.push_back(msgId);
        }
    }

    runNextJobIfIdle();
}

std::shared_ptr<Api::AsyncTask> SyncerImpl::asyncTask()
{
    return task_;
}

// must only be called while queueMutex_ is locked
void SyncerImpl::runNextJobIfIdle()
{
    if (!running_)
    {
        // downloading attachments has highest priority
        if (!messageAttachmentDownloadQueue_.empty())
        {
            auto msgId = messageAttachmentDownloadQueue_.front();
            messageAttachmentDownloadQueue_.pop_front();
            running_ = true;
            task_->setTask(downloadAttachmentsForMessageAsync(msgId));
        }
        else if (enqueuedSync_)
        {
            running_ = true;
            task_->setTask(runAsync(*enqueuedSync_));
            enqueuedSync_.reset();
        }
        else
        {
            if (auto listener = listener_) listener->finished();
        }
    }
}

std::shared_ptr<Api::AsyncTask> SyncerImpl::runAsync(Api::SyncMode mode)
{
    return std::make_shared<AsyncTaskImpl>(
                std::make_shared<SyncerRunWorker>(
                    sessionData_, mode, sessionListener_, taskSyncerListener_));
}

std::shared_ptr<Api::AsyncTask> SyncerImpl::downloadAttachmentsForMessageAsync(id_type msgId)
{
    return std::make_shared<AsyncTaskImpl>(
                std::make_shared<SyncerDownloadAttachmentsForMessageWorker>(
                    sessionData_, msgId, sessionListener_, taskSyncerListener_));
}

SyncerImpl::SyncerAsyncTask::SyncerAsyncTask(SyncerImpl &parent)
    : parent_(parent)
{}

void SyncerImpl::SyncerAsyncTask::cancel()
{
    std::lock_guard<std::recursive_mutex> lock(parent_.queueMutex_); K_RAII(lock);

    parent_.messageAttachmentDownloadQueue_.clear();
    parent_.enqueuedSync_.reset();
    task_ = nullptr;
    parent_.running_ = false;
}

bool SyncerImpl::SyncerAsyncTask::isDone()
{
    std::lock_guard<std::recursive_mutex> lock(parent_.queueMutex_); K_RAII(lock);

    // We're done if the queues are empty and there's no running task
    return
            parent_.messageAttachmentDownloadQueue_.empty() &&
            !parent_.enqueuedSync_ &&
            !parent_.running_;
}

void SyncerImpl::SyncerAsyncTask::waitUntilDone()
{
    while (!isDone())
    {
        if (auto task = task_)
        {
            task->waitUntilDone();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool SyncerImpl::SyncerAsyncTask::waitForMs(int32_t timeout)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto deadline = start + std::chrono::milliseconds(timeout);
    while (!isDone())
    {
        auto now = std::chrono::high_resolution_clock::now();
        if (now >= deadline) return false;

        if (auto task = task_)
        {
            auto timeout = deadline - now;
            auto timeoutMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
            task->waitForMs(timeoutMs);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    return true;
}

void SyncerImpl::SyncerAsyncTask::setTask(
        const std::shared_ptr<Api::AsyncTask> &task)
{
    task_ = task;
}

SyncerImpl::SyncerSyncerListener::SyncerSyncerListener(SyncerImpl &parent)
    : parent_(parent)
{}

void SyncerImpl::SyncerSyncerListener::draftAttachmentsTooBig(int64_t convId)
{
    if (auto listener = parent_.listener_) {
        listener->draftAttachmentsTooBig(convId);
    }
}

void SyncerImpl::SyncerSyncerListener::progressed(const Api::SyncProgress &progress)
{
    if (auto listener = parent_.listener_) {
        listener->progressed(progress);
    }
}

void SyncerImpl::SyncerSyncerListener::finished()
{
    std::lock_guard<std::recursive_mutex> lock(parent_.queueMutex_); K_RAII(lock);

    parent_.running_ = false;
    parent_.runNextJobIfIdle();
}

void SyncerImpl::SyncerSyncerListener::error(Api::NetworkError error)
{
    {
        std::lock_guard<std::recursive_mutex> lock(parent_.queueMutex_); K_RAII(lock);

        parent_.running_ = false;
        parent_.task_->cancel();
    }

    if (auto listener = parent_.listener_) listener->error(error);
}

}
}
