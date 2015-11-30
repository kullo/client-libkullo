/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <mutex>

#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api/Syncer.h"
#include "kulloclient/api_impl/sessiondata.h"

namespace Kullo {
namespace ApiImpl {

class SyncerImpl : public Api::Syncer
{
public:
    SyncerImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    std::shared_ptr<Api::AsyncTask> runAsync(
            Api::SyncMode mode,
            const std::shared_ptr<Api::SyncerRunListener> &listener) override;

    std::shared_ptr<Api::AsyncTask> downloadAttachmentsForMessageAsync(
            int64_t msgId,
            const std::shared_ptr<Api::SyncerRunListener> &listener
            ) override;

private:
    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;

    // lock this while syncing to ensure that only one syncer runs per session
    std::shared_ptr<std::mutex> syncMutex_;
};

}
}
