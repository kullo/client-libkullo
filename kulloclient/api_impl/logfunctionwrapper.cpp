/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/logfunctionwrapper.h"

#include "kulloclient/api/LogType.h"

namespace Kullo {
namespace ApiImpl {

using LibraryLogger = Kullo::Util::LibraryLogger;

LogFunctionWrapper::LogFunctionWrapper(
        const std::shared_ptr<Api::LogListener> &listener)
    : listener_(listener)
{
    using namespace std::placeholders;
    auto boundLogFunction = std::bind(
                &LogFunctionWrapper::logFunction, this,
                _1, _2, _3, _4, _5, _6, _7);
    LibraryLogger::setLogFunction(boundLogFunction);
}

LogFunctionWrapper::~LogFunctionWrapper()
{
    LibraryLogger::setLogFunction(LibraryLogger::defaultLogWrapper);
}

void LogFunctionWrapper::logFunction(
        const std::string &file,
        const int line,
        const std::string &function,
        const LibraryLogger::LogType type,
        const std::string &msg,
        const std::string &thread,
        const std::string &stacktrace)
{
    Api::LogType llType;
    switch (type)
    {
    case LibraryLogger::LogType::None:    llType = Api::LogType::None;    break;
    case LibraryLogger::LogType::Debug:   llType = Api::LogType::Debug;   break;
    case LibraryLogger::LogType::Info:    llType = Api::LogType::Info;    break;
    case LibraryLogger::LogType::Warning: llType = Api::LogType::Warning; break;
    case LibraryLogger::LogType::Error:   llType = Api::LogType::Error;   break;
    case LibraryLogger::LogType::Fatal:   llType = Api::LogType::Fatal;   break;
    default:
        kulloAssert(false);
        llType = Api::LogType::None;
    }

    listener_->writeLogMessage(
                file, line, function, llType, msg, thread, stacktrace);
}

}
}
