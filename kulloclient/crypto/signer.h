/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <vector>

#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/crypto/publickey.h"

namespace Kullo {
namespace Crypto {

class Signer
{
public:
    std::vector<unsigned char> sign(
            const std::vector<unsigned char> &data,
            const PrivateKey &key) const;

    bool verify(
            const std::vector<unsigned char> &data,
            const std::vector<unsigned char> &signature,
            const PublicKey &key) const;
};

}
}
