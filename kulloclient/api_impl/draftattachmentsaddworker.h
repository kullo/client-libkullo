/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>
#include <string>

#include "kulloclient/api/DraftAttachmentsAddListener.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace ApiImpl {

class DraftAttachmentsAddWorker : public Worker
{
public:
    DraftAttachmentsAddWorker(
            int64_t convId,
            const std::string &path,
            const std::string &mimeType,
            const Db::SessionConfig &sessionConfig,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::DraftAttachmentsAddListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    const int64_t convId_;
    const std::string path_;
    const std::string mimeType_;
    const Db::SessionConfig sessionConfig_;

    // all uses must be synchronized
    std::shared_ptr<Api::SessionListener> sessionListener_;
    std::shared_ptr<Api::DraftAttachmentsAddListener> listener_;
};

}
}
