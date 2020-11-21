/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
