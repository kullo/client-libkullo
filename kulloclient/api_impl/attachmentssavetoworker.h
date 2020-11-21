/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <string>

#include "kulloclient/api/LocalError.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace ApiImpl {

class AttachmentsSaveToWorker : public Worker
{
public:
    AttachmentsSaveToWorker(
            bool isDraft,
            int64_t convOrMsgId,
            int64_t attId,
            const std::string &path,
            const Db::SessionConfig &sessionConfig);

    void work() override;

protected:
    virtual void notifyListener(
            uint64_t convOrMsgId, uint64_t attId, const std::string &path) = 0;
    virtual void error(
            uint64_t convOrMsgId, uint64_t attId, const std::string &path,
            Api::LocalError error) = 0;

    // not synchronized, non-threadsafe stuff is only used from work()
    const bool isDraft_;
    const int64_t convOrMsgId_;
    const int64_t attId_;
    const std::string path_;
    const Db::SessionConfig sessionConfig_;
};

}
}
