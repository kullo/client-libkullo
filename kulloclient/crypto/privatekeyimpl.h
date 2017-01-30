/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
