/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Kullo {
namespace Util {

class Base64
{
public:
    static std::string encode(const std::string &str);
    static std::string encode(const std::vector<unsigned char> &data);
    static std::vector<unsigned char> decode(const std::string &str);
};

}
}
