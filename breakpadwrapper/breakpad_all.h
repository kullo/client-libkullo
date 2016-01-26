/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
