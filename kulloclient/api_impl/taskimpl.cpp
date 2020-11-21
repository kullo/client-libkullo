/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
