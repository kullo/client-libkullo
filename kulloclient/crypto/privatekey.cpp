/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/privatekey.h"

#include "kulloclient/crypto/privatekeyimpl.h"
#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Crypto {

PrivateKey::PrivateKey(
        AsymmetricKeyType type,
        std::shared_ptr<PrivateKeyImpl> priv)
    : type_(type), p(priv)
{
}

std::shared_ptr<PrivateKeyImpl> PrivateKey::priv() const
{
    return p;
}

AsymmetricKeyType PrivateKey::type() const
{
    return type_;
}

std::vector<unsigned char> PrivateKey::toVector() const
{
    kulloAssert(p);

    auto privkey = Botan::PKCS8::BER_encode(*p->privkey);
    return std::vector<unsigned char>(privkey.begin(), privkey.end());
}

PublicKey PrivateKey::pubkey() const
{
    auto key = std::shared_ptr<Botan::Public_Key>(
                Botan::X509::copy_key(*p->privkey));
    return PublicKey(type_, std::make_shared<PublicKeyImpl>(key));
}

bool PrivateKey::operator==(const PrivateKey &other) const
{
    if (!p || !other.p) return p == other.p;

    return *p == *other.p;
}

}
}
