/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/api/SyncMode.h"
#include "kulloclient/api_impl/syncerworker.h"

namespace Kullo {
namespace ApiImpl {

class SyncerDownloadAttachmentsForMessageWorker : public SyncerWorker
{
public:
    SyncerDownloadAttachmentsForMessageWorker(
            std::shared_ptr<SessionData> sessionData,
            id_type msgId,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::SyncerListener> listener);

protected:
    void doWork() override;

private:
    // all uses must be synchronized
    id_type msgId_;
};

}
}
