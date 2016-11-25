/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/scopedbenchmark.h"

#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/strings.h"

namespace Kullo {
namespace Util {

ScopedBenchmark::ScopedBenchmark(const std::string &title)
    : title_(title)
{
    start_ = std::chrono::steady_clock::now();
}

ScopedBenchmark::~ScopedBenchmark()
{
    auto end = std::chrono::steady_clock::now();
    auto runTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(end-start_).count();
    Log.d() << title_.c_str()
            << " (run time: " << Strings::formatReadable(runTimeMs).c_str() << "ms)";
}

}
}
