/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <chrono>
#include <string>

namespace Kullo {
namespace Util {

class ScopedBenchmark
{
public:
    ScopedBenchmark(const std::string &title);
    ~ScopedBenchmark();

private:
    std::string title_;
    std::chrono::steady_clock::time_point start_;
};

}
}
