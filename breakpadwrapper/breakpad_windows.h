/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <client/windows/handler/exception_handler.h>

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
            const wchar_t* dump_path,
            const wchar_t* minidump_id,
            void* context,
            EXCEPTION_POINTERS* exinfo,
            MDRawAssertionInfo* assertion,
            bool succeeded);

    google_breakpad::ExceptionHandler eh;
    CallbackContext m_context;
};

}
