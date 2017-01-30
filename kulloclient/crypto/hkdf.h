/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/crypto/symmetrickey.h"

namespace Kullo {
namespace Crypto {

class HKDF
{
public:
    static size_t const HASH_BITSIZE = 512;

    SymmetricKey expand(SymmetricKey const& input,
                        std::string const& info,
                        size_t bitLength) const;
};

}
}
