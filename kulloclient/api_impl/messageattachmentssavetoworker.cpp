/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/messageattachmentssavetoworker.h"

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
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    listener_.reset();
}

void MessageAttachmentsSaveToWorker::notifyListener(
        uint64_t convOrMsgId, uint64_t attId, const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    if (listener_)
    {
        listener_->finished(convOrMsgId, attId, path);
    }
}

void MessageAttachmentsSaveToWorker::error(
        uint64_t convOrMsgId, uint64_t attId, const std::string &path,
        Api::LocalError error)
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    if (listener_)
    {
        listener_->error(convOrMsgId, attId, path, error);
    }
}

}
}
