/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
