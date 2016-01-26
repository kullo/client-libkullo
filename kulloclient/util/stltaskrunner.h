/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <future>
#include <list>

#include "kulloclient/api/TaskRunner.h"

namespace Kullo {
namespace Util {

class StlTaskRunner: public Api::TaskRunner
{
public:
    /// Calls wait()
    ~StlTaskRunner() override;

    void runTaskAsync(const std::shared_ptr<Api::Task> &task) override;

    /**
     * @brief Wait for all tasks to terminate.
     *
     * This is necessary to prevent use after free when static variables are
     * destroyed in the main thread after main() has returned, but other threads
     * still use static variables.
     */
    void wait();

private:
    /**
     * @brief Delete all finished tasks.
     *
     * This should be called regularly to prune finished tasks from memory.
     */
    void pruneFinishedTasks();

    std::list<std::future<void>> futures_;
    bool waitCalled_ = false;
};

}
}
