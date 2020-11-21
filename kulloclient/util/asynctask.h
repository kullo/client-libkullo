/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <functional>
#include <thread>

namespace Kullo {
namespace Util {

/**
 * @brief Controls an asynchronously running task.
 *
 * Make sure that it is destroyed before any of the dependencies of its worker!
 * If for example the worker is a lambda that captures by reference any members
 * of the object that the AsyncTask is a member of, or if it captures the whole
 * object by capturing the "this" pointer, the AsyncTask should be the last
 * member of the object so that it is destroyed first.
 */
class AsyncTask final
{
public:
    using WorkerFunction = std::function<void()>;

    AsyncTask();

    /**
     * @brief Executes cancel() and wait() before destruction
     *
     * This makes sure that we have no stray threads running in the background
     * that could access resources of the foreground thread that might already
     * be freed.
     */
    ~AsyncTask();

    /**
     * @brief Runs the worker function asynchronously in the worker thread.
     *
     * If the worker of a previous invocation is still running, it cancels and
     * waits for it first.
     *
     * @param worker The function to be run asynchronously
     * @param onCancel The function to be called on cancel
     */
    void start(WorkerFunction worker, WorkerFunction onCancel);

    /// Runs the cancel function in the caller thread.
    void cancel();

    /// Waits for the worker thread to finish.
    void wait();

private:
    std::thread thread_;
    WorkerFunction onCancel_;
};

}
}
