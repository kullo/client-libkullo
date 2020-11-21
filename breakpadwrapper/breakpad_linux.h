/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#include <client/linux/handler/exception_handler.h>
#pragma clang diagnostic pop

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
