/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <thread>

#include "kulloclient/api/ClientCreateSessionListener.h"
#include "kulloclient/api/MasterKey.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/worker.h"

namespace Kullo {
namespace ApiImpl {

class ClientCreateSessionWorker : public Worker
{
public:
    ClientCreateSessionWorker(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::MasterKey> &masterKey,
            const std::string &dbFilePath,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::ClientCreateSessionListener> listener,
            const boost::optional<std::thread::id> mainThread);

    void work() override;
    void cancel() override;

    /// Synchronously creates a session
    std::shared_ptr<Api::Session> makeSession();

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    std::shared_ptr<Api::Address> address_;
    std::shared_ptr<Api::MasterKey> masterKey_;
    std::string dbFilePath_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    const boost::optional<std::thread::id> mainThread_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientCreateSessionListener> listener_;
};

}
}
