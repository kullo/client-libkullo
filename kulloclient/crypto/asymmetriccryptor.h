/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
