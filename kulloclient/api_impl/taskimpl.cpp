/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/taskimpl.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

TaskImpl::TaskImpl(const std::function<void()> &task)
    : task_(task)
{
    kulloAssert(task);
}

void TaskImpl::run()
{
    task_();
}

}
}
