/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <future>
#include <list>

#include "kulloclient/api/TaskRunner.h"

namespace Kullo {
namespace Util {

enum class StlTaskRunnerState {
    Active,
    Inactive
};

class StlTaskRunner: public Api::TaskRunner
{
public:
    /**
     * @brief Expects that wait() was called before destruction
     *
     * The task runner must be in inactive state on destruction, i.e. wait()
     * must be called.
     */
    ~StlTaskRunner() override;

    void runTaskAsync(const nn_shared_ptr<Api::Task> &task) override;

    /**
     * @brief Wait for all tasks to terminate.
     *
     * This is necessary to prevent use after free when static variables are
     * destroyed in the main thread after main() has returned, but other threads
     * still use static variables.
     */
    void wait();

    /**
      * @brief Resets the taskrunner tio active mode
      *
      * When wait() was called, the task runner went into inactive state. In this
      * state no further tasks must be added. This method sets the state back to
      * active allowing the reuse of the same task runner.
      */
    void reset();

private:
    /**
     * @brief Delete all finished tasks.
     *
     * This should be called regularly to prune finished tasks from memory.
     */
    void pruneFinishedTasks();

    std::list<std::future<void>> futures_;
    StlTaskRunnerState state_ = StlTaskRunnerState::Active;
};

}
}
