/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>
#include <string>

#include "kulloclient/api/DraftAttachmentsContentListener.h"
#include "kulloclient/api_impl/attachmentscontentworker.h"

namespace Kullo {
namespace ApiImpl {

class DraftAttachmentsContentWorker : public AttachmentsContentWorker
{
public:
    DraftAttachmentsContentWorker(
            int64_t convId,
            int64_t attId,
            const Db::SessionConfig &sessionConfig,
            std::shared_ptr<Api::DraftAttachmentsContentListener> listener);

    void cancel() override;

protected:
    void notifyListener(
            uint64_t convOrMsgId, uint64_t attId,
            const std::vector<uint8_t> &content) override;

private:
    // all uses must be synchronized
    std::shared_ptr<Api::DraftAttachmentsContentListener> listener_;
};

}
}
