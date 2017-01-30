/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <vector>

namespace Kullo {
namespace Util {

class GZip
{
public:
    static std::vector<unsigned char> compress(const std::vector<unsigned char> &data);
    static std::vector<unsigned char> decompress(const std::vector<unsigned char> &compressed);
};

}
}
