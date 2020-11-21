/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>
#include <thread>

#include "kulloclient/api/AsyncTask.h"
#include "kulloclient/api_impl/worker.h"

namespace Kullo {
namespace ApiImpl {

class AsyncTaskImpl : public Api::AsyncTask
{
public:
    AsyncTaskImpl(std::shared_ptr<Worker> worker);
    ~AsyncTaskImpl() override;

    void cancel() override;
    bool isDone() override;
    void waitUntilDone() override;
    bool waitForMs(int32_t timeout) override;

protected:
    std::weak_ptr<Worker> worker_;
};

}
}
