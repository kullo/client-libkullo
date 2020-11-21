/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>

#include "kulloclient/api/ClientAddressExistsListener.h"
#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/protocol/publickeysclient.h"

namespace Kullo {
namespace ApiImpl {

class ClientAddressExistsWorker : public Worker
{
public:
    ClientAddressExistsWorker(
            const Api::Address &address,
            std::shared_ptr<Api::ClientAddressExistsListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    Protocol::PublicKeysClient keysClient_;
    const Api::Address address_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientAddressExistsListener> listener_;
};

}
}
