/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/masterkey.h"

namespace Kullo {
namespace Crypto {

class SymmetricKeyGenerator
{
public:
    static size_t const MASTER_KEY_BITS = 256;
    static size_t const LOGIN_KEY_BITS = 512;
    static size_t const PRIVATE_DATA_KEY_BITS = 256;
    static size_t const MESSAGE_SYMM_KEY_BITS = 256;

    std::vector<unsigned char> makeRandomIV(size_t bytes) const;
    SymmetricKey makeMasterKey() const;
    SymmetricKey makeLoginKey(Util::KulloAddress const& user,
                              Util::MasterKey const& masterKey) const;
    SymmetricKey makePrivateDataKey() const;
    SymmetricKey makeMessageSymmKey() const;

private:
    std::shared_ptr<SymmetricKeyImpl> makeRandomKeyPriv(size_t bitSize) const;
};

}
}
