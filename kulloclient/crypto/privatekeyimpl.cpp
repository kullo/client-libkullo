/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/privatekeyimpl.h"

namespace Kullo {
namespace Crypto {

PrivateKeyImpl::PrivateKeyImpl(std::shared_ptr<Botan::Private_Key> key)
    : privkey(key)
{
}

bool PrivateKeyImpl::operator==(const PrivateKeyImpl &other) const
{
    if (!privkey || !other.privkey) return privkey == other.privkey;

    return Botan::PKCS8::BER_encode(*privkey) ==
           Botan::PKCS8::BER_encode(*other.privkey);
}

}
}
