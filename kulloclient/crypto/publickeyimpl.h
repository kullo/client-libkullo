/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>

#include <botan/botan_all.h>

namespace Kullo {
namespace Crypto {

struct PublicKeyImpl
{
    explicit PublicKeyImpl(std::shared_ptr<Botan::Public_Key> key);

    std::shared_ptr<Botan::Public_Key> pubkey;
};

}
}
