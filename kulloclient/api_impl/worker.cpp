/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/worker.h"

#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

void Worker::run()
{
    work();

    {
        std::lock_guard<std::mutex> lock(isFinishedMutex_); K_RAII(lock);
        isFinished_ = true;
    }
    isFinishedCv_.notify_one();
}

void Worker::wait()
{
    std::unique_lock<std::mutex> lock(isFinishedMutex_);
    isFinishedCv_.wait(lock, [this](){ return isFinished_; });
}

bool Worker::waitFor(std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(isFinishedMutex_);

    // The reason why we can't use a simple timed_mutex but need to use
    // a condition variable is that timed_mutex::wait_for times out early
    // on GCC 4.8 (Ubuntu 14.04) and on MSVC 12 / MSVS 2013.
    // See also https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54562
    return isFinishedCv_.wait_for(
                lock, timeout,
                [this](){ return isFinished_; });
}

bool Worker::isFinished()
{
    std::lock_guard<std::mutex> lock(isFinishedMutex_); K_RAII(lock);
    return isFinished_;
}

}
}
