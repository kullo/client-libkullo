/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

#include "kulloclient/api_impl/worker.h"

namespace Kullo {
namespace ApiImpl {

class AttachmentsContentWorker : public Worker
{
public:
    AttachmentsContentWorker(
            bool isDraft,
            int64_t convOrMsgId,
            int64_t attId,
            const std::string &dbPath);

    void work() override;

protected:
    virtual void notifyListener(
            uint64_t convOrMsgId, uint64_t attId,
            const std::vector<uint8_t> &content) = 0;

    // not synchronized, non-threadsafe stuff is only used from work()
    const bool isDraft_;
    const int64_t convOrMsgId_;
    const int64_t attId_;
    const std::string dbPath_;
};

}
}
