/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
