/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <deque>

#include "kulloclient/api/AsyncTask.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api/Syncer.h"
#include "kulloclient/api/SyncerListener.h"
#include "kulloclient/api/SyncMode.h"
#include "kulloclient/api_impl/DateTime.h"
#include "kulloclient/api_impl/sessiondata.h"

namespace Kullo {
namespace ApiImpl {

class SyncerImpl : public Api::Syncer
{
public:
    SyncerImpl(
            std::shared_ptr<SessionData> session,
            std::shared_ptr<Api::SessionListener> sessionListener);
    ~SyncerImpl() override;

    void setListener(const nn_shared_ptr<Api::SyncerListener> &listener)
        override;
    boost::optional<Api::DateTime> lastFullSync() override;
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
        SyncerSyncerListener(SyncerImpl *parent);
        void setParent(SyncerImpl *parent);

        // Api::SyncerListener implementation
        void started() override;
        void draftPartTooBig(
                int64_t convId, Api::DraftPart part,
                int64_t currentSize, int64_t maxSize) override;
        void progressed(const Api::SyncProgress & progress) override;
        void finished() override;
        void error(Api::NetworkError error) override;

    private:
        // must be hold while parent_ is accessed (both r/w)
        std::mutex parentMutex_;
        SyncerImpl *parent_;
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
    boost::optional<Api::DateTime> lastFullSync_;
    boost::optional<Api::SyncMode> lastSyncMode_;
};

}
}
