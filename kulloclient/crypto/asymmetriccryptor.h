/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/crypto/publickey.h"

namespace Kullo {
namespace Crypto {

class AsymmetricCryptor
{
public:
    virtual std::vector<unsigned char> encrypt(
            const std::vector<unsigned char> &plaintext,
            const PublicKey &key) const;

    virtual std::vector<unsigned char> decrypt(
            const std::vector<unsigned char> &ciphertext,
            const PrivateKey &key) const;

    virtual ~AsymmetricCryptor() = default;
};

}
}
