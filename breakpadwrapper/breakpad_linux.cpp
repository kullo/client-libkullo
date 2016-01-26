/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "breakpad_linux.h"

#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

namespace BreakpadWrapper {

static std::string getUserDataPath()
{
    const char *dataHome = std::getenv("XDG_DATA_HOME");
    if (dataHome) return std::string(dataHome);

    const char *home = std::getenv("HOME");
    if (home) return std::string(home) + "/.local/share";

    return std::string("/tmp");
}

static std::string getAndMakeDumpPath()
{
    std::string dumpPath = getUserDataPath() + "/Kullo";
    int result = mkdir(dumpPath.c_str(), S_IRWXU);
    if (result == 0 || errno == EEXIST)
    {
        return dumpPath;
    }
    else
    {
        return std::string("/tmp");
    }
}

BreakpadImpl::BreakpadImpl()
    : descriptor(getAndMakeDumpPath()),
      eh(descriptor, nullptr, &BreakpadImpl::dumpCallbackImpl, &m_context, true, -1)
{
}

BreakpadImpl::~BreakpadImpl()
{
}

void BreakpadImpl::registerCallback(dumpCallback callback, void *context)
{
    m_context.origCallback = callback;
    m_context.origContext = context;
}

void BreakpadImpl::dump()
{
    eh.WriteMinidump();
}

bool BreakpadImpl::dumpCallbackImpl(const google_breakpad::MinidumpDescriptor &descriptor, void *context, bool succeeded)
{
    std::cerr << std::boolalpha << "Dumped to: " << descriptor.path() << " (success: " << succeeded << ")" << std::endl;

    if (context != nullptr)
    {
        auto callbackContext = static_cast<CallbackContext*>(context);
        if (callbackContext->origCallback != nullptr)
        {
            return callbackContext->origCallback(descriptor.path(), callbackContext->origContext, succeeded);
        }
    }
    return succeeded;
}

}
