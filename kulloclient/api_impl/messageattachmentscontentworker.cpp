/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/messageattachmentscontentworker.h"

#include "kulloclient/util/multithreading.h"

namespace Kullo {
namespace ApiImpl {

MessageAttachmentsContentWorker::MessageAttachmentsContentWorker(
        int64_t msgId,
        int64_t attId,
        const Db::SessionConfig &sessionConfig,
        std::shared_ptr<Api::MessageAttachmentsContentListener> listener)
    : AttachmentsContentWorker(false, msgId, attId, sessionConfig)
    , listener_(listener)
{}

void MessageAttachmentsContentWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

void MessageAttachmentsContentWorker::notifyListener(
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
