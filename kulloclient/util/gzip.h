/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
