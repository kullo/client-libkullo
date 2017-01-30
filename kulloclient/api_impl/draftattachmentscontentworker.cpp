/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftattachmentscontentworker.h"

#include "kulloclient/util/misc.h"
#include "kulloclient/util/multithreading.h"

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsContentWorker::DraftAttachmentsContentWorker(
        int64_t convId,
        int64_t attId,
        const Db::SessionConfig &sessionConfig,
        std::shared_ptr<Api::DraftAttachmentsContentListener> listener)
    : AttachmentsContentWorker(true, convId, attId, sessionConfig)
    , listener_(listener)
{}

void DraftAttachmentsContentWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

void DraftAttachmentsContentWorker::notifyListener(
        uint64_t convOrMsgId, uint64_t attId,
        const std::vector<uint8_t> &content)
{
    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
    {
        listener->finished(convOrMsgId, attId, content);
    }
}

}
}
