/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/asymmetriccryptor.h"

#include <botan/botan_all.h>

#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/privatekeyimpl.h"
#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Botan;

namespace Kullo {
namespace Crypto {

namespace {
// EME = Encoding Method for Encryption
const std::string EME = "OAEP(SHA-512)";
}

std::vector<unsigned char> AsymmetricCryptor::encrypt(
        const std::vector<unsigned char> &plaintext,
        const PublicKey &key) const
{
    kulloAssert(key.type() == AsymmetricKeyType::Encryption);

    AutoSeeded_RNG rng;
    PK_Encryptor_EME enc(*key.p->pubkey, EME);
    return enc.encrypt(plaintext, rng);
}

std::vector<unsigned char> AsymmetricCryptor::decrypt(
        const std::vector<unsigned char> &ciphertext,
        const PrivateKey &key) const
{
    kulloAssert(key.type() == AsymmetricKeyType::Encryption);

    PK_Decryptor_EME dec(*key.p->privkey, EME);
    auto plaintext = dec.decrypt(ciphertext);
    return std::vector<unsigned char>(plaintext.cbegin(), plaintext.cend());
}

}
}
