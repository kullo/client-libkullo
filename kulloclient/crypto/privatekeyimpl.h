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

struct PrivateKeyImpl
{
    explicit PrivateKeyImpl(std::shared_ptr<Botan::Private_Key> key);

    bool operator==(const PrivateKeyImpl &other) const;

    std::shared_ptr<Botan::Private_Key> privkey;
};

}
}
