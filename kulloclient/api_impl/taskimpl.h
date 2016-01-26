/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
