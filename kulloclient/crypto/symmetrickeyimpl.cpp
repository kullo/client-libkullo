/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/symmetrickeyimpl.h"

namespace Kullo {
namespace Crypto {

SymmetricKeyImpl::SymmetricKeyImpl(const Botan::SymmetricKey &key)
    : key(key)
{
}

bool SymmetricKeyImpl::operator==(const SymmetricKeyImpl &other) const
{
    return this->key.bits_of() == other.key.bits_of();
}

std::ostream &operator<<(std::ostream &out, const SymmetricKeyImpl &symmKey)
{
    out << symmKey.key.as_string();
    return out;
}

}
}
