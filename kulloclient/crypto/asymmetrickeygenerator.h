/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
