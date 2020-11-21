/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/crypto/signer.h"

#include <botan/botan_all.h>

#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/privatekeyimpl.h"
#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/assert.h"

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

    Botan::AutoSeeded_RNG rng;
    Botan::PK_Signer sig(*key.p->privkey, rng, EMSA);
    return sig.sign_message(data, rng);
}

bool Signer::verify(
        const std::vector<unsigned char> &data,
        const std::vector<unsigned char> &signature,
        const PublicKey &key) const
{
    kulloAssert(key.type() == AsymmetricKeyType::Signature);

    Botan::PK_Verifier ver(*key.p->pubkey, EMSA);
    return ver.verify_message(data, signature);
}

}
}
