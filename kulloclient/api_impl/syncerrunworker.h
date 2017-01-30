/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/SyncMode.h"
#include "kulloclient/api_impl/syncerworker.h"

namespace Kullo {
namespace ApiImpl {

class SyncerRunWorker : public SyncerWorker
{
public:
    SyncerRunWorker(
            std::shared_ptr<SessionData> sessionData,
            Api::SyncMode mode,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::SyncerListener> listener);

protected:
    void doWork() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    Api::SyncMode mode_;
};

}
}
