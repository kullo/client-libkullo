/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace Kullo {
namespace ApiImpl {

class Worker
{
public:
    Worker() = default;
    virtual ~Worker() = default;

    virtual void work() = 0;
    virtual void cancel() = 0;

    void run();
    void wait();
    bool waitFor(std::chrono::milliseconds timeout);
    bool isFinished();

protected:
    // As long as you don't own this mutex, you may only do thread-safe stuff to
    // all data in this class!
    std::mutex mutex_;

private:
    std::mutex isFinishedMutex_;
    std::condition_variable isFinishedCv_;
    bool isFinished_ = false;
};

}
}
