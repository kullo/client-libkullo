/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/hex.h"

#include <botan/botan_all.h>

namespace Kullo {
namespace Util {

std::string Hex::encode(const std::string &str)
{
    return encode(std::vector<unsigned char>(str.cbegin(), str.cend()));
}

std::string Hex::encode(const std::vector<unsigned char> &data)
{
    return Botan::hex_encode(data, false);
}

std::vector<unsigned char> Hex::decode(const std::string &str)
{
    return Botan::hex_decode(str);
}

}
}
