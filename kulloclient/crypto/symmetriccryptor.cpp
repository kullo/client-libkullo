/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/symmetriccryptor.h"

#include <vector>
#include <botan/botan.h>

#include "kulloclient/crypto/symmetrickeyimpl.h"
#include "kulloclient/util/assert.h"

using namespace Botan;

namespace Kullo {
namespace Crypto {

const std::string SymmetricCryptor::CONTENT_IV     = "content";
const std::string SymmetricCryptor::ATTACHMENTS_IV = "attachments";
const size_t SymmetricCryptor::RANDOM_IV_BYTES = 128 / 8;

namespace {
const std::string CIPHER = "AES-256/GCM";
}

std::vector<unsigned char> SymmetricCryptor::encrypt(
        const std::vector<unsigned char> &plaintext,
        const SymmetricKey &key,
        const std::vector<unsigned char> &iv,
        PrependIV prependIV) const
{
    auto result = crypt(plaintext, key, iv, ENCRYPT);
    if (prependIV == PrependIV::True)
    {
        result.insert(result.begin(), iv.cbegin(), iv.cend());
    }
    return result;
}

std::vector<unsigned char> SymmetricCryptor::decrypt(
        const std::vector<unsigned char> &ivAndCiphertext,
        const SymmetricKey &key,
        size_t ivLength) const
{
    kulloAssert(ivAndCiphertext.size() >= ivLength);

    auto ciphertextBegin = ivAndCiphertext.cbegin() + ivLength;
    std::vector<unsigned char> iv(ivAndCiphertext.cbegin(), ciphertextBegin);
    std::vector<unsigned char> ciphertext(ciphertextBegin, ivAndCiphertext.cend());
    return crypt(ciphertext, key, iv, DECRYPT);
}

std::vector<unsigned char> SymmetricCryptor::decrypt(
        const std::vector<unsigned char> &ciphertext,
        const SymmetricKey &key,
        const std::vector<unsigned char> &iv) const
{
    return crypt(ciphertext, key, iv, DECRYPT);
}

std::vector<unsigned char> SymmetricCryptor::crypt(
        const std::vector<unsigned char> &input,
        const SymmetricKey &key,
        const std::vector<unsigned char> &iv,
        SymmetricCryptor::Direction direction) const
{
    if (!input.size()) return std::vector<unsigned char>();

    Cipher_Dir dir = ENCRYPTION;  // initialization to make the compiler happy
    switch (direction)
    {
    case ENCRYPT:
        dir = ENCRYPTION;
        break;
    case DECRYPT:
        dir = DECRYPTION;
        break;
    default:
        kulloAssert(false);
    }

    Pipe pipe(get_cipher(CIPHER, key.priv()->key, iv, dir));
    pipe.process_msg(input);
    secure_vector<byte> output = pipe.read_all(0);
    return std::vector<unsigned char>(output.cbegin(), output.cend());
}

}
}
