/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftattachmentssavetoworker.h"

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsSaveToWorker::DraftAttachmentsSaveToWorker(
        int64_t convId, int64_t attId,
        const std::string &path, const std::string &dbPath,
        std::shared_ptr<Api::DraftAttachmentsSaveToListener> listener)
    : AttachmentsSaveToWorker(true, convId, attId, path, dbPath)
    , listener_(listener)
{}

void DraftAttachmentsSaveToWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    listener_.reset();
}

void DraftAttachmentsSaveToWorker::notifyListener(
        uint64_t convOrMsgId, uint64_t attId, const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    if (listener_)
    {
        listener_->finished(convOrMsgId, attId, path);
    }
}

void DraftAttachmentsSaveToWorker::error(
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
