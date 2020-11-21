/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/binary.h"

namespace Kullo {
namespace Util {

std::vector<unsigned char> to_vector(const std::string &str)
{
    return std::vector<unsigned char>(str.cbegin(), str.cend());
}

std::string to_string(const std::vector<unsigned char> &vec)
{
    return std::string(vec.cbegin(), vec.cend());
}

}
}
