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
#include "kulloclient/crypto/asymmetrickeytype.h"

namespace Kullo {
namespace Crypto {

class PublicKey
{
public:
    PublicKey(AsymmetricKeyType type, std::shared_ptr<PublicKeyImpl> priv);
    std::shared_ptr<PublicKeyImpl> priv() const;

    AsymmetricKeyType type() const;
    std::vector<unsigned char> toVector() const;

private:
    AsymmetricKeyType type_ = AsymmetricKeyType::Invalid;
    std::shared_ptr<PublicKeyImpl> p;

    friend class AsymmetricCryptor;
    friend class Signer;
};

}
}
