/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/stltaskrunner.h"

#include "kulloclient/api/Task.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"

#include <chrono>

namespace Kullo {
namespace Util {

namespace {
const auto MAX_TASKS_COUNT = 25;
}

StlTaskRunner::~StlTaskRunner()
{
    kulloAssert(waitCalled_ == true);

    wait();
}

void StlTaskRunner::runTaskAsync(const std::shared_ptr<Api::Task> &task)
{
    auto future = std::async(
                std::launch::async,
                // Bind shared_ptr to task by value so that the task is not
                // freed until its run method has returned.
                [task](){ task->run(); }
            );
    futures_.emplace_back(std::move(future));

    // Task is running, now check if we can delete something
    auto taskCountBefore = futures_.size();
    if (taskCountBefore > MAX_TASKS_COUNT)
    {
        Log.d() << "Pruning tasks ...";
        auto start = std::chrono::high_resolution_clock::now();
        pruneFinishedTasks();
        auto end = std::chrono::high_resolution_clock::now();
        auto taskCountAfter = futures_.size();

        Log.d() << "Deleted " << taskCountBefore - taskCountAfter << " taks in "
                << std::chrono::duration<double, std::milli>(end - start).count()
                << " ms";

        if (taskCountAfter > MAX_TASKS_COUNT)
        {
            Log.w() << "Task count still exceeds maximum after pruning "
                    << "(count: " << taskCountAfter
                    << ", max: " << MAX_TASKS_COUNT << ")";
        }
    }
}

void StlTaskRunner::wait()
{
    waitCalled_ = true;

    for (auto &future : futures_)
    {
        if (future.valid()) future.wait();
    }
    futures_.clear();
}

void StlTaskRunner::pruneFinishedTasks()
{
    auto futureIter = futures_.begin();
    while (futureIter != futures_.end())
    {
        // wait_for() must only be called on valid futures
        if (futureIter->valid())
        {
            // don't wait, just check
            auto status = futureIter->wait_for(std::chrono::milliseconds(0));
            if (status == std::future_status::ready)
            {
                futureIter = futures_.erase(futureIter);
                continue;
            }
        }

        ++futureIter;
    }
}

}
}
