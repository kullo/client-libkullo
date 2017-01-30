/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>
#include <memory>

#include "kulloclient/api/LogListener.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

class LogFunctionWrapper final
{
public:
    LogFunctionWrapper(const std::shared_ptr<Api::LogListener> &listener);
    ~LogFunctionWrapper();

private:
    void logFunction(
            const std::string &file,
            const int line,
            const std::string &function,
            const Util::LibraryLogger::LogType type,
            const std::string &msg,
            const std::string &thread,
            const std::string &stacktrace);

    std::shared_ptr<Api::LogListener> listener_;
};

}
}
