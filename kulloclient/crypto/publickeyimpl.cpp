/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/publickeyimpl.h"

namespace Kullo {
namespace Crypto {

PublicKeyImpl::PublicKeyImpl(std::shared_ptr<Botan::Public_Key> key)
    : pubkey(key)
{
}

}
}
