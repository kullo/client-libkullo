/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/messageattachmentscontentworker.h"

namespace Kullo {
namespace ApiImpl {

MessageAttachmentsContentWorker::MessageAttachmentsContentWorker(
        int64_t msgId,
        int64_t attId,
        const std::string &dbPath,
        std::shared_ptr<Api::MessageAttachmentsContentListener> listener)
    : AttachmentsContentWorker(false, msgId, attId, dbPath)
    , listener_(listener)
{}

void MessageAttachmentsContentWorker::cancel()
{
    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    listener_.reset();
}

void MessageAttachmentsContentWorker::notifyListener(
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
