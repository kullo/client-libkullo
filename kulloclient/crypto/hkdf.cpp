/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/hkdf.h"

#include <botan/botan_all.h>

#include "kulloclient/crypto/symmetrickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Crypto {

SymmetricKey HKDF::expand(const SymmetricKey &input,
                          const std::string &info,
                          size_t bitLength) const
{
    // don't accept small inputs
    kulloAssert(input.bitSize() >= 128);

    kulloAssert(bitLength % 8 == 0);

    auto expander = Botan::KDF::create(
                std::string("HKDF-Expand(HMAC(SHA-") +
                std::to_string(HASH_BITSIZE) +
                "))");
    kulloAssert(expander);

    auto key = expander->derive_key(
                bitLength / 8,
                input.priv()->key.bits_of(),
                info);
    return SymmetricKey(std::make_shared<SymmetricKeyImpl>(key));
}

}
}
