// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from syncer.djinni

#pragma once

#include <boost/optional.hpp>
#include <cstdint>
#include <kulloclient/nn.h>
#include <memory>

namespace Kullo { namespace Api {

class SyncerListener;
enum class SyncMode;
struct DateTime;

/**
 * Handles syncing, including downloading of attachments. Prevents multiple syncs
 * from running in parallel by building a queue and intelligently merging sync
 * requests.
 * Examples: Enqueuing a WithoutAttachments sync removes a SendOnly sync from the
 * queue. A running SendOnly sync would be cancelled. Enqueuing a SendOnly sync
 * while a WithoutAttachments sync is running or enqueued will do nothing.
 * Enqueuing an Everything sync will remove all attachment download requests from
 * the queue.
 */
class Syncer {
public:
    virtual ~Syncer() {}

    /**
     * Set or replace the SyncerListener which should receive sync events.
     *
     * Thread-safe.
     */
    virtual void setListener(const ::Kullo::nn_shared_ptr<SyncerListener> & listener) = 0;

    /**
     * Get the finishing time of the last successful full sync.
     * A full sync is one with "WithoutAttachments" or "Everything" sync mode.
     * Returns null if there hasn't been a sync yet.
     *
     * Thread-safe.
     */
    virtual boost::optional<DateTime> lastFullSync() = 0;

    /**
     * Request that the data specified in mode is synced.
     *
     * Thread-safe.
     */
    virtual void requestSync(SyncMode mode) = 0;

    /**
     * Request that all attachments for the given message are downloaded.
     *
     * Thread-safe.
     */
    virtual void requestDownloadingAttachmentsForMessage(int64_t msgId) = 0;

    /**
     * Cancels the running sync and enqueued syncs, but doesn't wait for
     * termination. Stops all callbacks, even if the task continues to run.
     *
     * Thread-safe.
     */
    virtual void cancel() = 0;

    /**
     * Returns true iff a sync is currently running.
     *
     * Thread-safe.
     */
    virtual bool isSyncing() = 0;

    /**
     * Blocks until the running sync and all enqueued syncs have finished.
     *
     * Thread-safe.
     */
    virtual void waitUntilDone() = 0;

    /**
     * Blocks until the sync and all enqueued syncs have finished executing or
     * until the timeout has expired. Returns false on timeout, true otherwise.
     *
     * Thread-safe.
     */
    virtual bool waitForMs(int32_t timeout) = 0;
};

} }  // namespace Kullo::Api
