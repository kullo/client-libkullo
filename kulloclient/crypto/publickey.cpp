/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/publickey.h"

#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Crypto {

PublicKey::PublicKey(AsymmetricKeyType type, std::shared_ptr<PublicKeyImpl> priv)
    : type_(type),
      p(priv)
{
}

std::shared_ptr<PublicKeyImpl> PublicKey::priv() const
{
    return p;
}

AsymmetricKeyType PublicKey::type() const
{
    return type_;
}

std::vector<unsigned char> PublicKey::toVector() const
{
    kulloAssert(p);

    auto pubkey = p->pubkey->public_key_bits();
    return std::vector<unsigned char>(pubkey.begin(), pubkey.end());
}

}
}
