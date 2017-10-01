/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/api/ClientCheckCredentialsListener.h"
#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/MasterKey.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/protocol/messagesclient.h"

namespace Kullo {
namespace ApiImpl {

class ClientCheckCredentialsWorker : public Worker
{
public:
    ClientCheckCredentialsWorker(
            const Api::Address &address,
            const Api::MasterKey &masterKey,
            std::shared_ptr<Api::ClientCheckCredentialsListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    Protocol::MessagesClient messagesClient_;
    const Api::Address address_;
    const Api::MasterKey masterKey_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientCheckCredentialsListener> listener_;
};

}
}
