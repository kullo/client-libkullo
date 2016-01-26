/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>

#include <botan/botan.h>

namespace Kullo {
namespace Crypto {

struct PublicKeyImpl
{
    explicit PublicKeyImpl(std::shared_ptr<Botan::Public_Key> key);

    std::shared_ptr<Botan::Public_Key> pubkey;
};

}
}
