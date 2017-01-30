/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <vector>

#include "kulloclient/crypto/symmetrickey.h"

namespace Kullo {
namespace Crypto {

enum struct PrependIV { False, True };

/// Symmetrically encrypts and decrypts data
class SymmetricCryptor
{
public:
    /// IV for content
    static const std::string CONTENT_IV;

    /// IV for attachments
    static const std::string ATTACHMENTS_IV;

    /// IV length in bytes for random IVs
    static const size_t RANDOM_IV_BYTES;

    /// Encrypts the given plaintext with the given key and IV (AES-256/GCM)
    virtual std::vector<unsigned char> encrypt(
            const std::vector<unsigned char> &plaintext,
            const SymmetricKey &key,
            const std::vector<unsigned char> &iv,
            PrependIV prependIV) const;

    /// Decrypts the given plaintext with the given key and the IV with the given length (bytes) from the beginning of the data (AES-256/GCM)
    virtual std::vector<unsigned char> decrypt(
            const std::vector<unsigned char> &ivAndCiphertext,
            const SymmetricKey &key,
            size_t ivLength) const;

    /// Decrypts the given plaintext with the given key and IV (AES-256/GCM)
    virtual std::vector<unsigned char> decrypt(
            const std::vector<unsigned char> &ciphertext,
            const SymmetricKey &key,
            const std::vector<unsigned char> &iv) const;

    virtual ~SymmetricCryptor() = default;
};

}
}
