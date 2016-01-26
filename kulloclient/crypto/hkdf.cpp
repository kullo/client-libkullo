/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/hkdf.h"

#include <botan/botan.h>

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

    // see RFC 5869, 2.3, "L"
    kulloAssert(bitLength <= 255 * HASH_BITSIZE);

    // N = ceil(L/HashLen)
    size_t n = bitLength / HASH_BITSIZE;
    if (HASH_BITSIZE * n < bitLength) ++n;

    // T = T(1) | T(2) | T(3) | ... | T(N)
    // T(0) = empty string (zero length)
    // T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
    // ...
    Botan::secure_vector<Botan::byte> t;
    Botan::secure_vector<Botan::byte> lastT;

    auto prk = input.priv()->key;
    Botan::Pipe hmacPipe(new Botan::MAC_Filter(std::string("HMAC(SHA-")
                                               + std::to_string(HASH_BITSIZE)
                                               + ")",
                                               prk));

    for (size_t i = 1; i <= n; ++i) {
        hmacPipe.start_msg();
        hmacPipe.write(lastT);
        hmacPipe.write(info);
        hmacPipe.write(static_cast<Botan::byte>(i));
        hmacPipe.end_msg();
        lastT = hmacPipe.read_all(Botan::Pipe::LAST_MESSAGE);
        t += lastT;
    }

    // OKM = first L octets of T
    if (t.size() > bitLength / 8) t.resize(bitLength / 8);

    return SymmetricKey(std::make_shared<SymmetricKeyImpl>(t));
}

}
}
