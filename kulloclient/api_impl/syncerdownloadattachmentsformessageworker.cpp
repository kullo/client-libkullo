/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/syncerdownloadattachmentsformessageworker.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

SyncerDownloadAttachmentsForMessageWorker::SyncerDownloadAttachmentsForMessageWorker(
        std::shared_ptr<SessionData> sessionData,
        id_type msgId,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::SyncerListener> listener)
    : SyncerWorker(sessionData, sessionListener, listener)
    , msgId_(msgId)
{}

void SyncerDownloadAttachmentsForMessageWorker::doWork()
{
    syncer_.downloadAttachmentsForMessage(msgId_, shouldCancel_);
}

}
}
