/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/asymmetrickeygenerator.h"

#include <botan/botan_all.h>

#include "kulloclient/crypto/privatekeyimpl.h"

namespace Kullo {
namespace Crypto {

namespace {
const size_t KEY_BITS = 4096;
}

PrivateKey AsymmetricKeyGenerator::makeKeyPair(AsymmetricKeyType type) const
{
    Botan::AutoSeeded_RNG rng;
    auto priv = std::make_shared<PrivateKeyImpl>(
                std::make_shared<Botan::RSA_PrivateKey>(rng, KEY_BITS));
    return PrivateKey(type, priv);
}

}
}
