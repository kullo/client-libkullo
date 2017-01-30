/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <vector>

#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/crypto/publickey.h"

namespace Kullo {
namespace Crypto {

class AsymmetricKeyLoader
{
public:
    PrivateKey loadUnencryptedPrivateKey(AsymmetricKeyType type, const std::vector<unsigned char> &data) const;
    PublicKey loadPublicKey(AsymmetricKeyType type, const std::vector<unsigned char> &data) const;

    std::vector<unsigned char> toVector(const PrivateKey &privkey) const;
    std::vector<unsigned char> toVector(const PublicKey &pubkey) const;

    virtual ~AsymmetricKeyLoader() = default;
};

}
}
