/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/publickey.h"

#include "kulloclient/crypto/publickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Crypto {

PublicKey::PublicKey(AsymmetricKeyType type, std::shared_ptr<PublicKeyImpl> priv)
    : type_(type),
      p(priv)
{
}

std::shared_ptr<PublicKeyImpl> PublicKey::priv() const
{
    return p;
}

AsymmetricKeyType PublicKey::type() const
{
    return type_;
}

std::vector<unsigned char> PublicKey::toVector() const
{
    kulloAssert(p);

    auto pubkey = p->pubkey->x509_subject_public_key();
    return std::vector<unsigned char>(pubkey.begin(), pubkey.end());
}

}
}
