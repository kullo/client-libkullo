/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <string>

#include "kulloclient/api/DraftAttachmentsSaveToListener.h"
#include "kulloclient/api_impl/attachmentssavetoworker.h"

namespace Kullo {
namespace ApiImpl {

class DraftAttachmentsSaveToWorker : public AttachmentsSaveToWorker
{
public:
    DraftAttachmentsSaveToWorker(
            int64_t convId,
            int64_t attId,
            const std::string &path,
            const std::string &dbPath,
            std::shared_ptr<Api::DraftAttachmentsSaveToListener> listener);

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
    std::shared_ptr<Api::DraftAttachmentsSaveToListener> listener_;
};

}
}
