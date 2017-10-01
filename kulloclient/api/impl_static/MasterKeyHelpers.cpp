/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/MasterKeyHelpers.h"

#include <stdexcept>
#include <boost/algorithm/string.hpp>

#include "kulloclient/api_impl/MasterKey.h"
#include "kulloclient/crypto/exceptions.h"
#include "kulloclient/crypto/pbkdf.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/util/base64.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/numeric_cast.h"

namespace Kullo {
namespace Api {

namespace {
const uint32_t ARGON2_TIMES = 2;
const uint32_t ARGON2_MEMORY_KIB = 1<<16;
const uint32_t ARGON2_PARALLELISM = 2;
const uint32_t ARGON2_HASH_BYTES = 256/8;
}

boost::optional<MasterKey> MasterKeyHelpers::createFromPem(const std::string &pem)
{
    try
    {
        auto key = Util::MasterKey(pem);
        return Api::MasterKey(key);
    }
    catch (...)
    {
        return boost::none;
    }
}

boost::optional<MasterKey> MasterKeyHelpers::createFromDataBlocks(
        const std::vector<std::string> &dataBlocks)
{
    try
    {
        return Api::MasterKey(dataBlocks);
    }
    catch (...)
    {
        return boost::none;
    }
}

bool MasterKeyHelpers::isValidBlock(const std::string &block)
{
    return Util::MasterKey::isPlausibleSingleBlock(block);
}

std::string MasterKeyHelpers::toPem(const MasterKey &masterKey)
{
    auto key = Util::MasterKey(masterKey.blocks);
    return key.toPem();
}

namespace {
struct EncryptedMasterKey
{
    std::string algorithm;
    uint32_t argon2MemoryKiB = 0;
    uint32_t argon2Times = 0;
    uint32_t argon2Parallelism = 0;
    std::vector<unsigned char> argon2Salt;
    std::vector<unsigned char> ciphertext;

    EncryptedMasterKey() = default;

    EncryptedMasterKey(const std::string &encryptedMasterKey)
    {
        std::vector<std::string> parts;
        boost::split(parts, encryptedMasterKey, boost::is_any_of("_"));
        if (parts.size() != 6) throw std::invalid_argument("!= 5 args");

        algorithm = parts[0];
        argon2MemoryKiB = Util::numeric_cast<uint32_t>(std::stol(parts[1]));
        argon2Times = Util::numeric_cast<uint32_t>(std::stol(parts[2]));
        argon2Parallelism = Util::numeric_cast<uint32_t>(std::stol(parts[3]));
        argon2Salt = Util::Base64::decode(parts[4]);
        ciphertext = Util::Base64::decode(parts[5]);
    }

    std::string toString() const
    {
        return algorithm +
                "_" + std::to_string(argon2MemoryKiB) +
                "_" + std::to_string(argon2Times) +
                "_" + std::to_string(argon2Parallelism) +
                "_" + Util::Base64::encode(argon2Salt) +
                "_" + Util::Base64::encode(ciphertext);
    }
};
}

std::string MasterKeyHelpers::encrypt(
        const std::string &password, const MasterKey &masterKey)
{
    EncryptedMasterKey encrypted;
    encrypted.algorithm = "argon2i";
    encrypted.argon2MemoryKiB = ARGON2_MEMORY_KIB;
    encrypted.argon2Times = ARGON2_TIMES;
    encrypted.argon2Parallelism = ARGON2_PARALLELISM;

    auto keygen = Crypto::SymmetricKeyGenerator();
    encrypted.argon2Salt = keygen.makeRandomIV(16);

    auto rawKey = Crypto::PBKDF::argon2i(
                encrypted.argon2Times,
                encrypted.argon2MemoryKiB,
                encrypted.argon2Parallelism,
                password,
                encrypted.argon2Salt,
                ARGON2_HASH_BYTES);
    auto key = Crypto::SymmetricKey(rawKey);

    auto rawPlaintext = Util::MasterKey(masterKey.blocks).toVector();
    auto iv = keygen.makeRandomIV(Crypto::SymmetricCryptor::RANDOM_IV_BYTES);
    auto cryptor = Crypto::SymmetricCryptor();
    encrypted.ciphertext =
            cryptor.encrypt(rawPlaintext, key, iv, Crypto::PrependIV::True);

    return encrypted.toString();
}

boost::optional<MasterKey> MasterKeyHelpers::decrypt(
        const std::string &password, const std::string &encryptedMasterKey)
{
    auto encrypted = EncryptedMasterKey(encryptedMasterKey);
    if (encrypted.algorithm != "argon2i")
    {
        throw std::invalid_argument(
                    std::string("Bad algorithm: ") + encrypted.algorithm);
    }

    auto rawKey = Crypto::PBKDF::argon2i(
                encrypted.argon2Times,
                encrypted.argon2MemoryKiB,
                encrypted.argon2Parallelism,
                password,
                encrypted.argon2Salt,
                ARGON2_HASH_BYTES);
    auto key = Crypto::SymmetricKey(rawKey);

    auto cryptor = Crypto::SymmetricCryptor();
    std::vector<unsigned char> rawPlaintext;
    try
    {
        rawPlaintext = cryptor.decrypt(
                    encrypted.ciphertext, key,
                    Crypto::SymmetricCryptor::RANDOM_IV_BYTES);
    }
    catch (Crypto::IntegrityFailure &)
    {
        return boost::none;
    }

    return MasterKey(Util::MasterKey(rawPlaintext));
}

}
}
