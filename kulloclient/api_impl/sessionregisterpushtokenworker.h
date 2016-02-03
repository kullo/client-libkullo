/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api_impl/worker.h"
#include "kulloclient/protocol/pushclient.h"

namespace Kullo {
namespace ApiImpl {

class SessionRegisterPushTokenWorker : public Worker
{
public:
    enum Operation { Add, Remove };

    SessionRegisterPushTokenWorker(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            Operation operation,
            const std::string &registrationToken);

    void work() override;
    void cancel() override;

private:
    // const stuff, synchronization unnecessary
    const Operation operation_;

    // synchronized by cancelMutex_
    std::mutex cancelMutex_;
    std::condition_variable cancelCv_;
    bool cancelled_ = false;

    // not synchronized, non-threadsafe stuff is only used from work()
    Protocol::PushClient pushClient_;
    std::string registrationToken_;
};

}
}
