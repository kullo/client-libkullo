/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
