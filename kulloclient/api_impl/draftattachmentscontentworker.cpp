/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/draftattachmentscontentworker.h"

namespace Kullo {
namespace ApiImpl {

DraftAttachmentsContentWorker::DraftAttachmentsContentWorker(
        int64_t convId,
        int64_t attId,
        const std::string &dbPath,
        std::shared_ptr<Api::DraftAttachmentsContentListener> listener)
    : AttachmentsContentWorker(true, convId, attId, dbPath)
    , listener_(listener)
{}

void DraftAttachmentsContentWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    listener_.reset();
}

void DraftAttachmentsContentWorker::notifyListener(
        uint64_t convOrMsgId, uint64_t attId,
        const std::vector<uint8_t> &content)
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    if (listener_)
    {
        listener_->finished(convOrMsgId, attId, content);
    }
}

}
}
