/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <botan/botan.h>

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
