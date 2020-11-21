/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/draftattachmentssavetoworker.h"

#include "kulloclient/util/multithreading.h"

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsSaveToWorker::DraftAttachmentsSaveToWorker(
        int64_t convId, int64_t attId,
        const std::string &path, const Db::SessionConfig &sessionConfig,
        std::shared_ptr<Api::DraftAttachmentsSaveToListener> listener)
    : AttachmentsSaveToWorker(true, convId, attId, path, sessionConfig)
    , listener_(listener)
{}

void DraftAttachmentsSaveToWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

void DraftAttachmentsSaveToWorker::notifyListener(
        uint64_t convOrMsgId, uint64_t attId, const std::string &path)
{
    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
    {
        listener->finished(convOrMsgId, attId, path);
    }
}

void DraftAttachmentsSaveToWorker::error(
        uint64_t convOrMsgId, uint64_t attId, const std::string &path,
        Api::LocalError error)
{
    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
    {
        listener->error(convOrMsgId, attId, path, error);
    }
}

}
}
