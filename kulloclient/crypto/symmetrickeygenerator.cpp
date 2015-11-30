/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/symmetrickeygenerator.h"

#include <botan/botan.h>

#include "kulloclient/crypto/hkdf.h"
#include "kulloclient/crypto/symmetrickeyimpl.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Crypto {

std::vector<unsigned char> SymmetricKeyGenerator::makeRandomIV(size_t bytes) const
{
    Botan::AutoSeeded_RNG rng;
    auto iv = Botan::OctetString(rng, bytes);
    return std::vector<unsigned char>(iv.begin(), iv.end());
}

SymmetricKey SymmetricKeyGenerator::makeMasterKey() const
{
    return SymmetricKey(makeRandomKeyPriv(MASTER_KEY_BITS));
}

SymmetricKey SymmetricKeyGenerator::makeLoginKey(const Util::KulloAddress &user,
                                                 const Util::MasterKey &masterKey) const
{
    HKDF hkdf;
    std::string info = std::string("login^") + user.toString();
    return hkdf.expand(SymmetricKeyLoader::fromMasterKey(masterKey), info, LOGIN_KEY_BITS);
}

SymmetricKey SymmetricKeyGenerator::makePrivateDataKey() const
{
    return SymmetricKey(makeRandomKeyPriv(PRIVATE_DATA_KEY_BITS));
}

SymmetricKey SymmetricKeyGenerator::makeMessageSymmKey() const
{
    return SymmetricKey(makeRandomKeyPriv(MESSAGE_SYMM_KEY_BITS));
}

std::shared_ptr<SymmetricKeyImpl> SymmetricKeyGenerator::makeRandomKeyPriv(
        size_t bitSize) const
{
    kulloAssert(bitSize%8 == 0);
    Botan::AutoSeeded_RNG rng;
    auto key = Botan::SymmetricKey(rng, bitSize / 8);

    return std::make_shared<SymmetricKeyImpl>(key);
}

}
}
