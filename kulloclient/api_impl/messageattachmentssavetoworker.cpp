/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/messageattachmentssavetoworker.h"

#include "kulloclient/util/multithreading.h"

namespace Kullo {
namespace ApiImpl {

MessageAttachmentsSaveToWorker::MessageAttachmentsSaveToWorker(
        int64_t convId, int64_t attId,
        const std::string &path, const Db::SessionConfig &sessionConfig,
        std::shared_ptr<Api::MessageAttachmentsSaveToListener> listener)
    : AttachmentsSaveToWorker(false, convId, attId, path, sessionConfig)
    , listener_(listener)
{}

void MessageAttachmentsSaveToWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

void MessageAttachmentsSaveToWorker::notifyListener(
        uint64_t convOrMsgId, uint64_t attId, const std::string &path)
{
    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
    {
        listener->finished(convOrMsgId, attId, path);
    }
}

void MessageAttachmentsSaveToWorker::error(
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
