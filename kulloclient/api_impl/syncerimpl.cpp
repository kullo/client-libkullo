/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/syncerimpl.h"

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
    , syncMutex_(std::make_shared<std::mutex>())
{
    kulloAssert(sessionData_);
}

std::shared_ptr<Api::AsyncTask> SyncerImpl::runAsync(
        Api::SyncMode mode,
        const std::shared_ptr<Api::SyncerRunListener> &listener)
{
    kulloAssert(listener);

    return std::make_shared<AsyncTaskImpl>(
                std::make_shared<SyncerRunWorker>(
                    sessionData_, mode, syncMutex_, sessionListener_, listener));
}

std::shared_ptr<Api::AsyncTask> SyncerImpl::downloadAttachmentsForMessageAsync(
        int64_t msgId, const std::shared_ptr<Api::SyncerRunListener> &listener)
{
    kulloAssert(msgId > 0);
    kulloAssert(listener);

    return std::make_shared<AsyncTaskImpl>(
                std::make_shared<SyncerDownloadAttachmentsForMessageWorker>(
                    sessionData_, msgId, syncMutex_, sessionListener_, listener));
    return nullptr;
}

}
}
