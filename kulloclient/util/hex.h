/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

namespace Kullo {
namespace Util {

class Hex
{
public:
    static std::string encode(const std::string &str);
    static std::string encode(const std::vector<unsigned char> &data);
    static std::vector<unsigned char> decode(const std::string &str);
};

}
}
