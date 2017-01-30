/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <memory>

#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api/SyncerListener.h"
#include "kulloclient/api/SyncPhase.h"
#include "kulloclient/api_impl/sessiondata.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/sync/syncer.h"

namespace Kullo {
namespace ApiImpl {

class SyncerWorker : public Worker
{
public:
    SyncerWorker(
            std::shared_ptr<SessionData> sessionData,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::SyncerListener> listener);

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
    std::shared_ptr<Api::SyncerListener> safeListener();

    // all uses must be synchronized
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::shared_ptr<Api::SyncerListener> listener_;
};

inline Api::SyncPhase implToApi(Sync::SyncPhase phase) {
    switch (phase) {
    case Sync::SyncPhase::Keys:
        return Api::SyncPhase::Keys;
    case Sync::SyncPhase::Profile:
        return Api::SyncPhase::Profile;
    case Sync::SyncPhase::IncomingMessages:
        return Api::SyncPhase::IncomingMessages;
    case Sync::SyncPhase::IncomingAttachments:
        return Api::SyncPhase::IncomingAttachments;
    case Sync::SyncPhase::OutgoingMessages:
        return Api::SyncPhase::OutgoingMessages;
    }
}

}
}
