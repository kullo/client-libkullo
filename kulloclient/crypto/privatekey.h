/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/crypto/publickey.h"

namespace Kullo {
namespace Crypto {

class PrivateKey
{
public:
    PrivateKey(AsymmetricKeyType type, std::shared_ptr<PrivateKeyImpl> priv);
    std::shared_ptr<PrivateKeyImpl> priv() const;

    AsymmetricKeyType type() const;
    std::vector<unsigned char> toVector() const;
    PublicKey pubkey() const;

    bool operator==(const PrivateKey &other) const;

private:
    AsymmetricKeyType type_ = AsymmetricKeyType::Invalid;
    std::shared_ptr<PrivateKeyImpl> p;

    friend class AsymmetricCryptor;
    friend class Signer;
};

}
}
