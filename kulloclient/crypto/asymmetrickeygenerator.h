/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/crypto/privatekey.h"

namespace Kullo {
namespace Crypto {

class AsymmetricKeyGenerator
{
public:
    PrivateKey makeKeyPair(AsymmetricKeyType type) const;
};

}
}
