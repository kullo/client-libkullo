/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/base64.h"

#include <botan/botan.h>

namespace Kullo {
namespace Util {

std::string Base64::encode(const std::string &str)
{
    return encode(std::vector<unsigned char>(str.cbegin(), str.cend()));
}

std::string Base64::encode(const std::vector<unsigned char> &data)
{
    return Botan::base64_encode(data);
}

std::vector<unsigned char> Base64::decode(const std::string &str)
{
    bool ignore_ws = true;
    Botan::secure_vector<Botan::byte> output = Botan::base64_decode(str, ignore_ws);
    return std::vector<unsigned char>(output.cbegin(), output.cend());
}

}
}
