/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <memory>

#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api/SyncerRunListener.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/sync/syncer.h"

namespace Kullo {
namespace ApiImpl {

class SyncerWorker : public Worker
{
public:
    SyncerWorker(
            std::shared_ptr<std::mutex> syncMutex,
            std::shared_ptr<SessionData> sessionData,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::SyncerRunListener> listener);

    void work() override;
    void cancel() override;

protected:
    virtual void doWork() = 0;

    // not synchronized, non-threadsafe stuff is only used from work()
    std::shared_ptr<std::atomic<bool>> shouldCancel_;
    Sync::Syncer syncer_;

private:
    void setupEvents();
    void sendEvent(const std::shared_ptr<Api::InternalEvent> &event);

    // not synchronized, non-threadsafe stuff is only used from work()
    std::shared_ptr<std::mutex> syncMutex_;
    std::shared_ptr<SessionData> sessionData_;

    // all uses must be synchronized
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::shared_ptr<Api::SyncerRunListener> listener_;
};

}
}
