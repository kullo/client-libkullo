/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <botan/botan_all.h>

namespace Kullo {
namespace Crypto {

struct SymmetricKeyImpl
{
    explicit SymmetricKeyImpl(Botan::SymmetricKey const& key);
    bool operator==(const SymmetricKeyImpl &other) const;

    Botan::SymmetricKey key;

    friend std::ostream &operator<<(std::ostream &out,
                                    const SymmetricKeyImpl &symmKey);
};

}
}
