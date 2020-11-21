/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <thread>
#include <boost/optional.hpp>

#include "kulloclient/api/ClientCreateSessionListener.h"
#include "kulloclient/api/SessionListener.h"
#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/MasterKey.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/kulloclient-forwards.h"

namespace Kullo {
namespace ApiImpl {

class ClientCreateSessionWorker : public Worker
{
public:
    ClientCreateSessionWorker(
            const Api::Address &address,
            const Api::MasterKey &masterKey,
            const std::string &dbFilePath,
            std::shared_ptr<Api::SessionListener> sessionListener,
            std::shared_ptr<Api::ClientCreateSessionListener> listener,
            const boost::optional<std::thread::id> mainThread);

    void work() override;
    void cancel() override;

    /// Synchronously creates a session
    nn_shared_ptr<Api::Session> makeSession();

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    const Api::Address address_;
    const Api::MasterKey masterKey_;
    std::string dbFilePath_;
    std::shared_ptr<Api::SessionListener> sessionListener_;
    const boost::optional<std::thread::id> mainThread_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientCreateSessionListener> listener_;
};

}
}
