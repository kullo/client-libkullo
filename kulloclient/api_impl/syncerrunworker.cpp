/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/syncerrunworker.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

SyncerRunWorker::SyncerRunWorker(
        std::shared_ptr<SessionData> sessionData,
        Api::SyncMode mode,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::SyncerListener> listener)
    : SyncerWorker(sessionData, sessionListener, listener)
    , mode_(mode)
{}

namespace {

Sync::SyncMode toSyncSyncMode(Api::SyncMode mode)
{
    switch (mode)
    {
    case Api::SyncMode::SendOnly:
        return Sync::SyncMode::SendOnly;
    case Api::SyncMode::WithoutAttachments:
        return Sync::SyncMode::WithoutAttachments;
    case Api::SyncMode::Everything:
        return Sync::SyncMode::Everything;
    default:
        kulloAssert(false);
        return Sync::SyncMode::Everything;
    }
}

}

void SyncerRunWorker::doWork()
{
    syncer_.run(toSyncSyncMode(mode_), shouldCancel_);
}

}
}
