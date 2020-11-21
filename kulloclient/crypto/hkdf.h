/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
