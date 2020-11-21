/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/api/SessionAccountInfoListener.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/protocol/accountclient.h"

namespace Kullo {
namespace ApiImpl {

class SessionAccountInfoWorker : public Worker
{
public:
    SessionAccountInfoWorker(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            std::shared_ptr<Api::SessionAccountInfoListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    Protocol::AccountClient accountClient_;

    // all uses must be synchronized
    std::shared_ptr<Api::SessionAccountInfoListener> listener_;
};

}
}
