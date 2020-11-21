/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>

namespace BreakpadWrapper {

typedef bool dumpCallback(const char *path, void *context, bool succeeded);

class BreakpadImpl;

class Breakpad final
{
public:
    static Breakpad *getInstance();
    ~Breakpad();

    void registerCallback(dumpCallback callback, void *context = nullptr);
    void dump();

private:
    Breakpad();

    static std::unique_ptr<Breakpad> s_breakpad;
    std::unique_ptr<BreakpadImpl> m_impl;
};

}
