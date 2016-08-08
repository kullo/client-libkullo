/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/stltaskrunner.h"

#include "kulloclient/api/Task.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/scopedbenchmark.h"

#include <chrono>

namespace Kullo {
namespace Util {

namespace {
// number of tasks that will trigger pruning
const auto PRUNING_THRESHOLD = 25;
}

StlTaskRunner::~StlTaskRunner()
{
    kulloAssert(waitCalled_ == true);
    kulloAssert(futures_.empty());
}

void StlTaskRunner::runTaskAsync(const std::shared_ptr<Api::Task> &task)
{
    if (waitCalled_) return;

    auto future = std::async(
                std::launch::async,
                // Bind shared_ptr to task by value so that the task is not
                // freed until its run method has returned.
                [task](){ task->run(); }
            );
    futures_.emplace_back(std::move(future));

    // Task is running, now check if we should prune futures
    auto taskCountBefore = futures_.size();
    if (taskCountBefore > PRUNING_THRESHOLD)
    {
        Log.d() << "Pruning tasks ...";
        auto start = std::chrono::high_resolution_clock::now();
        pruneFinishedTasks();
        auto end = std::chrono::high_resolution_clock::now();
        auto taskCountAfter = futures_.size();

        Log.d() << "Pruned " << taskCountBefore - taskCountAfter << " tasks in "
                << std::chrono::duration<double, std::milli>(end - start).count()
                << " ms";

        if (taskCountAfter > PRUNING_THRESHOLD)
        {
            Log.w() << "Task count still exceeds threshold after pruning "
                    << "(count: " << taskCountAfter
                    << ", threshold: " << PRUNING_THRESHOLD << ")";
        }
    }
}

void StlTaskRunner::wait()
{
    kulloAssert(waitCalled_ == false);
    waitCalled_ = true;

    ScopedBenchmark benchmark("Waiting for tasks to complete"); K_RAII(benchmark);

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
