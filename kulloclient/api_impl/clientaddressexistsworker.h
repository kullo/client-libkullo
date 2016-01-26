/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/api/Address.h"
#include "kulloclient/api/ClientAddressExistsListener.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/protocol/publickeysclient.h"

namespace Kullo {
namespace ApiImpl {

class ClientAddressExistsWorker : public Worker
{
public:
    ClientAddressExistsWorker(
        std::shared_ptr<Api::Address> address,
        std::shared_ptr<Api::ClientAddressExistsListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    Protocol::PublicKeysClient keysClient_;
    std::shared_ptr<Api::Address> address_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientAddressExistsListener> listener_;
};

}
}
