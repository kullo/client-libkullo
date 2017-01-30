/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <iostream>
#include <string>
#include <kulloclient/util/assert.h>
#include <kulloclient/util/exceptions.h>

namespace Kullo {
namespace Crypto {

enum struct AsymmetricKeyType {Invalid, Encryption, Signature};

/// AsymmetricKeyType to string conversion. Values "enc" and "sig"
/// are fixed by the Kullo protocol and must not change.
inline std::string toString(AsymmetricKeyType type)
{
    if (type == AsymmetricKeyType::Encryption) return "enc";
    if (type == AsymmetricKeyType::Signature) return "sig";

    kulloAssertionFailed("AsymmetricKeyType must be valid");
}

inline AsymmetricKeyType keyTypeFromString(const std::string &type)
{
    if (type == "enc") return AsymmetricKeyType::Encryption;
    if (type == "sig") return AsymmetricKeyType::Signature;

    throw Util::InvalidArgument("Key type string has invalid value.");
}

}
}

inline std::ostream &operator<<(std::ostream &out, Kullo::Crypto::AsymmetricKeyType type)
{
    out << Kullo::Crypto::toString(type);
    return out;
}
