/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/symmetrickey.h"

#include <botan/botan_all.h>

#include "kulloclient/crypto/symmetrickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Crypto {

SymmetricKey::SymmetricKey(const std::string &hexStr)
    : p(std::make_shared<SymmetricKeyImpl>(Botan::SymmetricKey(hexStr)))
{
}

SymmetricKey::SymmetricKey(std::shared_ptr<SymmetricKeyImpl> priv)
    : p(std::move(priv))
{
}

std::shared_ptr<SymmetricKeyImpl> SymmetricKey::priv() const
{
    return p;
}

size_t SymmetricKey::bitSize() const
{
    kulloAssert(p);

    return p->key.length() * 8;
}

std::string SymmetricKey::toBase64() const
{
    kulloAssert(p);

    return Botan::base64_encode(p->key.bits_of());
}

std::string SymmetricKey::toHex() const
{
    kulloAssert(p);

    return Botan::hex_encode(p->key.bits_of(), false);
}

std::vector<unsigned char> SymmetricKey::toVector() const
{
    kulloAssert(p);

    auto keyData = p->key.bits_of();
    auto vec = std::vector<unsigned char>(keyData.begin(), keyData.end());
    return vec;
}

bool SymmetricKey::operator==(const SymmetricKey &other) const
{
    if (!p || !other.p)
    {
        return p == other.p;
    }

    return *p == *other.p;
}

std::ostream &operator<<(std::ostream &out, const SymmetricKey &symmKey)
{
    if (symmKey.p) out << *symmKey.p;
    return out;
}

}
}
