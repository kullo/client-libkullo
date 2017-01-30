/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/asymmetrickeyloader.h"

#include <botan/botan_all.h>

#include "kulloclient/crypto/privatekeyimpl.h"
#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/base64.h"

using namespace Botan;
using namespace Kullo::Util;

namespace Kullo {
namespace Crypto {

PrivateKey AsymmetricKeyLoader::loadUnencryptedPrivateKey(AsymmetricKeyType type, const std::vector<unsigned char> &data) const
{
    AutoSeeded_RNG rng;

    std::string wrapped;
    wrapped  = "-----BEGIN PRIVATE KEY-----\n";
    wrapped += Base64::encode(data);
    wrapped += "\n-----END PRIVATE KEY-----";
    auto wrappedData = reinterpret_cast<const unsigned char*>(wrapped.data());
    DataSource_Memory privkeySrc(wrappedData, wrapped.size());
    return PrivateKey(type, std::make_shared<PrivateKeyImpl>(
                          std::shared_ptr<Botan::Private_Key>(
                              PKCS8::load_key(privkeySrc, rng))));
}

PublicKey AsymmetricKeyLoader::loadPublicKey(AsymmetricKeyType type, const std::vector<unsigned char> &data) const
{
    return PublicKey(
                type,
                std::make_shared<PublicKeyImpl>(
                    std::shared_ptr<Botan::Public_Key>(X509::load_key(data))));
}

std::vector<unsigned char> AsymmetricKeyLoader::toVector(const PrivateKey &privkey) const
{
    auto encoded = Botan::PKCS8::BER_encode(*privkey.priv()->privkey);
    return std::vector<unsigned char>(encoded.cbegin(), encoded.cend());
}

std::vector<unsigned char> AsymmetricKeyLoader::toVector(const PublicKey &pubkey) const
{
    return Botan::X509::BER_encode(*pubkey.priv()->pubkey);
}

}
}
