/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include "kulloclient/api/Address.h"
#include "kulloclient/api/MasterKey.h"
#include "kulloclient/api/ClientCheckLoginListener.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/protocol/messagesclient.h"

namespace Kullo {
namespace ApiImpl {

class ClientCheckLoginWorker : public Worker
{
public:
    ClientCheckLoginWorker(
            std::shared_ptr<Api::Address> address,
            std::shared_ptr<Api::MasterKey> masterKey,
            std::shared_ptr<Api::ClientCheckLoginListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    Protocol::MessagesClient messagesClient_;
    std::shared_ptr<Api::Address> address_;
    std::shared_ptr<Api::MasterKey> masterKey_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientCheckLoginListener> listener_;
};

}
}
