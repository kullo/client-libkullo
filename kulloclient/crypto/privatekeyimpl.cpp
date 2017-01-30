/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
