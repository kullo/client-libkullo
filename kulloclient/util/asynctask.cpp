/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/asynctask.h"

namespace Kullo {
namespace Util {

namespace {
    AsyncTask::WorkerFunction noop = [](){};
}

AsyncTask::AsyncTask()
    : onCancel_(noop)
{
}

AsyncTask::~AsyncTask()
{
    cancel();
    wait();
}

void AsyncTask::start(WorkerFunction worker, WorkerFunction onCancel)
{
    cancel();
    wait();

    onCancel_ = onCancel;
    thread_ = std::thread(worker);
}

void AsyncTask::cancel()
{
    onCancel_();
}

void AsyncTask::wait()
{
    if (thread_.joinable()) thread_.join();
}

}
}
