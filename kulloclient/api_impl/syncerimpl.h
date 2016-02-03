/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <deque>

#include "kulloclient/api/AsyncTask.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api/Syncer.h"
#include "kulloclient/api/SyncerListener.h"
#include "kulloclient/api_impl/sessiondata.h"

namespace Kullo {
namespace ApiImpl {

class SyncerImpl : public Api::Syncer
{
public:
    SyncerImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);

    void setListener(const std::shared_ptr<Api::SyncerListener> &listener)
        override;
    void requestSync(Api::SyncMode mode) override;
    void requestDownloadingAttachmentsForMessage(int64_t msgId) override;
    void cancel() override;
    bool isSyncing() override;
    void waitUntilDone() override;
    bool waitForMs(int32_t timeout) override;

private:
    class SyncerSyncerListener : public Api::SyncerListener
    {
    public:
        SyncerSyncerListener(SyncerImpl &parent);

        // Api::SyncerListener implementation
        void started() override;
        void draftAttachmentsTooBig(int64_t convId) override;
        void progressed(const Api::SyncProgress & progress) override;
        void finished() override;
        void error(Api::NetworkError error) override;

    private:
        SyncerImpl &parent_;
    };

    void runNextJobIfIdle();
    std::shared_ptr<Api::AsyncTask> runAsync(Api::SyncMode mode);
    std::shared_ptr<Api::AsyncTask> downloadAttachmentsForMessageAsync(id_type msgId);

    std::shared_ptr<SessionData> sessionData_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::shared_ptr<Api::SyncerListener> listener_;

    std::shared_ptr<Api::AsyncTask> task_;
    std::shared_ptr<SyncerSyncerListener> taskSyncerListener_;
    std::recursive_mutex queueMutex_;

    // members only to be accessed when queueMutex_ is held
    boost::optional<Api::SyncMode> enqueuedSync_;
    std::deque<id_type> messageAttachmentDownloadQueue_;
    bool running_;
};

}
}
