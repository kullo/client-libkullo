/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <client/linux/handler/exception_handler.h>

#include "breakpad_all.h"

namespace BreakpadWrapper {

class BreakpadImpl
{
public:
    BreakpadImpl();
    ~BreakpadImpl();

    void registerCallback(dumpCallback callback, void *context);
    void dump();

private:
    struct CallbackContext
    {
        dumpCallback *origCallback = nullptr;
        void *origContext = nullptr;
    };

    static bool dumpCallbackImpl(
            const google_breakpad::MinidumpDescriptor& descriptor,
            void* context,
            bool succeeded);

    google_breakpad::MinidumpDescriptor descriptor;
    google_breakpad::ExceptionHandler eh;
    CallbackContext m_context;
};

}
