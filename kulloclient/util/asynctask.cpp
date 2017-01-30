/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
