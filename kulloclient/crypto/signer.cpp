/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/signer.h"

#include <botan/botan.h>

#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/privatekeyimpl.h"
#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Botan;

namespace Kullo {
namespace Crypto {

namespace {
// EMSA = Encoding Method for Signature with Appendix
const std::string EMSA = "PSSR(SHA-512)";
}

std::vector<unsigned char> Signer::sign(
        const std::vector<unsigned char> &data,
        const PrivateKey &key) const
{
    kulloAssert(key.type() == AsymmetricKeyType::Signature);

    AutoSeeded_RNG rng;
    PK_Signer sig(*key.p->privkey, EMSA);
    return sig.sign_message(data, rng);
}

bool Signer::verify(
        const std::vector<unsigned char> &data,
        const std::vector<unsigned char> &signature,
        const PublicKey &key) const
{
    kulloAssert(key.type() == AsymmetricKeyType::Signature);

    PK_Verifier ver(*key.p->pubkey, EMSA);
    return ver.verify_message(data, signature);
}

}
}
