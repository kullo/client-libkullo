/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/Registry.h"

#include "kulloclient/registry.h"

namespace Kullo {
namespace Api {

void Registry::setLogListener(const std::shared_ptr<LogListener> &listener)
{
    Kullo::Registry::setLogListener(listener);
}

void Registry::setTaskRunner(const std::shared_ptr<TaskRunner> &taskRunner)
{
    Kullo::Registry::setTaskRunner(taskRunner);
}

}
}
