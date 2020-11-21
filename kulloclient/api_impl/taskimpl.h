/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <functional>

#include "kulloclient/api/Task.h"

namespace Kullo {
namespace ApiImpl {

class TaskImpl final: public Api::Task
{
public:
    TaskImpl(const std::function<void()> &task);

    void run() override;

private:
    std::function<void()> task_;
};

}
}
