/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/asynctaskimpl.h"

#include <chrono>

#include "kulloclient/registry.h"
#include "kulloclient/api_impl/taskimpl.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

AsyncTaskImpl::AsyncTaskImpl(std::shared_ptr<Worker> worker)
    : worker_(worker)
{
    kulloAssert(worker);

    Registry::taskRunner()->runTaskAsync(
                std::make_shared<TaskImpl>(
                    // Capture worker by value (which creates a copy, increasing
                    // the reference count of the shared_ptr) to make it live at
                    // least as long as the worker thread is running. If there's
                    // no reference to it outside of AsyncTaskImpl, it will live
                    // exactly until the worker thread terminates, because
                    // AsyncTaskImpl only retains a weak_ptr.
                    [worker]()
    {
        try
        {
            worker->run();
        }
        catch (std::exception &ex)
        {
            Log.e() << "Uncaught exception in AsyncTask: "
                    << Util::formatException(ex);
        }
    }));
}

AsyncTaskImpl::~AsyncTaskImpl()
{
    cancel();
}

void AsyncTaskImpl::cancel()
{
    if (auto workerLocked = worker_.lock())
    {
        workerLocked->cancel();
    }
}

bool AsyncTaskImpl::isDone()
{
    if (auto workerLocked = worker_.lock())
    {
        return workerLocked->isFinished();
    }
    else
    {
        // When the worker has already been deallocated, it must be finished.
        return true;
    }
}

void AsyncTaskImpl::waitUntilDone()
{
    if (auto workerLocked = worker_.lock())
    {
        workerLocked->wait();
    }
}

bool AsyncTaskImpl::waitForMs(int32_t timeout)
{
    if (auto workerLocked = worker_.lock())
    {
        return workerLocked->waitFor(std::chrono::milliseconds(timeout));
    }

    // If worker cannot be locked, it is already done (or cancelled), so this
    // is a success.
    return true;
}

}
}
