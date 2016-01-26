/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/asymmetrickeygenerator.h"

#include <botan/botan.h>

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
