/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/api/PushToken.h"
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
            const Api::PushToken &token);

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
    Api::PushToken token_;
};

}
}
