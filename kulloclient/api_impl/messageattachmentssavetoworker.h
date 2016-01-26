/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <string>

#include "kulloclient/api/MessageAttachmentsSaveToListener.h"
#include "kulloclient/api_impl/attachmentssavetoworker.h"

namespace Kullo {
namespace ApiImpl {

class MessageAttachmentsSaveToWorker : public AttachmentsSaveToWorker
{
public:
    MessageAttachmentsSaveToWorker(
            int64_t msgId,
            int64_t attId,
            const std::string &path,
            const std::string &dbPath,
            std::shared_ptr<Api::MessageAttachmentsSaveToListener> listener);

    void cancel() override;

protected:
    void notifyListener(
            uint64_t convOrMsgId, uint64_t attId, const std::string &path
            ) override;
    void error(
            uint64_t convOrMsgId, uint64_t attId, const std::string &path,
            Api::LocalError error
            ) override;

private:
    // all uses must be synchronized
    std::shared_ptr<Api::MessageAttachmentsSaveToListener> listener_;
};

}
}
