/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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
