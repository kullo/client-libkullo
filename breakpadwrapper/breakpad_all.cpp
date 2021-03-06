/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "breakpad_all.h"

#if defined _WIN32
    #include "breakpad_windows.h"
#elif defined __linux
    #include "breakpad_linux.h"
#else
    #error "Unsupported OS"
#endif

namespace BreakpadWrapper {

std::unique_ptr<Breakpad> Breakpad::s_breakpad;

Breakpad *Breakpad::getInstance()
{
    if (!s_breakpad) s_breakpad.reset(new Breakpad());
    return s_breakpad.get();
}

Breakpad::~Breakpad()
{
}

void Breakpad::registerCallback(dumpCallback callback, void *context)
{
    m_impl->registerCallback(callback, context);
}

void Breakpad::dump()
{
    m_impl->dump();
}

Breakpad::Breakpad()
    : m_impl(new BreakpadImpl)
{
}

}
