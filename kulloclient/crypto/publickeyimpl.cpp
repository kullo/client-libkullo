/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/publickeyimpl.h"

namespace Kullo {
namespace Crypto {

PublicKeyImpl::PublicKeyImpl(std::shared_ptr<Botan::Public_Key> key)
    : pubkey(key)
{
}

}
}
