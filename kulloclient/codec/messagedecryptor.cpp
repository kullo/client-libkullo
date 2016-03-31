/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/messagedecryptor.h"

#include <stdexcept>
#include <vector>
#include <jsoncpp/jsoncpp.h>

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/checkedconverter.h"
#include "kulloclient/util/jsonhelper.h"
#include "kulloclient/util/limits.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/scopedbenchmark.h"

namespace Kullo {
namespace Codec {

MessageDecryptor::MessageDecryptor(
        const Protocol::Message &message,
        std::shared_ptr<PrivateKeyProvider> privKeyProvider,
        const Crypto::SymmetricKey &privateDataKey,
        Crypto::AsymmetricCryptor *asymmCryptor,
        Crypto::SymmetricCryptor *symmCryptor)
    : message_(message)
    , privKeyProvider_(privKeyProvider)
    , privateDataKey_(privateDataKey)
    , asymmCryptor_(asymmCryptor)
    , symmCryptor_(symmCryptor)
{
    kulloAssert(!message_.deleted);

    if (!asymmCryptor) asymmCryptor_.reset(new Crypto::AsymmetricCryptor);
    if (!symmCryptor) symmCryptor_.reset(new Crypto::SymmetricCryptor);
}

DecryptedMessage MessageDecryptor::decrypt() const
{
    checkSize();

    DecryptedMessage result;
    result.id = message_.id;
    result.lastModified = message_.lastModified;
    result.dateReceived = message_.dateReceived;

    try
    {
        result.keySafe = parseKeySafe(decryptKeySafe());
    }
    catch (Util::ConversionException&)
    {
        std::throw_with_nested(
                    InvalidContentFormat("MessageDecryptor::decrypt()"));
    }

    result.content = decryptContent(result.keySafe);
    std::tie(result.metaVersion, result.meta) = decryptMeta();

    return std::move(result);
}

namespace {

unsigned int readUint32(const std::vector<unsigned char> &data,
                               size_t offset)
{
    unsigned int result = 0;
    result |= data[offset] << 24;
    result |= data[offset + 1] << 16;
    result |= data[offset + 2] << 8;
    result |= data[offset + 3];
    return result;
}

}

std::string MessageDecryptor::decryptKeySafe() const
{
    // keySafe (without encKeyId) must be non-empty
    if (message_.keySafe.size() <= 4)
    {
        throw InvalidContentFormat(
                    std::string("Failed to decrypt message ")
                    + std::to_string(message_.id)
                    + ": keySafe too short");
    }

    auto encKeyId = readUint32(message_.keySafe, 0);

    std::vector<unsigned char> encryptedKeySafe(
                message_.keySafe.cbegin() + 4,
                message_.keySafe.cend());

    try
    {
        auto privkey = privKeyProvider_->getKey(
                      Crypto::AsymmetricKeyType::Encryption,
                      encKeyId);

        auto benchmark = Util::ScopedBenchmark("Decrypt keysafe of message " + std::to_string(message_.id));
        K_RAII(benchmark);
        auto keySafe = asymmCryptor_->decrypt(encryptedKeySafe, privkey);
        return std::string(keySafe.begin(), keySafe.end());
    }
    catch (Db::DatabaseIntegrityError)
    {
        std::throw_with_nested(DecryptionKeyMissing());
        return "";  // silence "missing return" warning
    }
}

DecryptedContent MessageDecryptor::decryptContent(const KeySafe &keySafe) const
{
    auto content = symmCryptor_->decrypt(
                message_.content,
                keySafe.symmKey,
                Util::to_vector(Crypto::SymmetricCryptor::CONTENT_IV)
                );

    if (content.size() < 8)
    {
        throw InvalidContentFormat(
                    std::string("Failed to decrypt message ")
                    + std::to_string(message_.id)
                    + ": decrypted data too short");
    }

    DecryptedContent result;
    result.sigKeyId = readUint32(content, 0);
    auto sigLen = readUint32(content, 4);

    if (content.size() - 8 < sigLen)
    {
        throw InvalidContentFormat(
                    std::string("Failed to decrypt message ")
                    + std::to_string(message_.id)
                    + ": decrypted data too short [2]");
    }

    std::copy(content.cbegin() + 8,
              content.cbegin() + 8 + sigLen,
              std::back_inserter(result.signature));

    std::copy(content.cbegin() + 8 + sigLen,
              content.cend(),
              std::back_inserter(result.data));
    return result;
}

std::tuple<metaVersion_type, std::string> MessageDecryptor::decryptMeta() const
{
    if (message_.meta.size() < 4) return std::make_pair(LATEST_META_VERSION, "");

    std::vector<unsigned char> ciphertext(message_.meta.cbegin() + 4, message_.meta.cend());
    std::vector<unsigned char> plaintext;

    metaVersion_type version = readUint32(message_.meta, 0);
    switch (version)
    {
    case 0:
        plaintext = symmCryptor_->decrypt(
                    ciphertext,
                    privateDataKey_,
                    Util::to_vector("meta"));
        break;

    case 1:
        plaintext = symmCryptor_->decrypt(
                    ciphertext,
                    privateDataKey_,
                    Crypto::SymmetricCryptor::RANDOM_IV_BYTES);
        break;

    default:
        kulloAssert(version > LATEST_META_VERSION);  // not all cases have been handled
        // if the version is not supported, we return a pair (version, "")
        break;
    }

    std::string result(plaintext.begin(), plaintext.end());
    return std::make_tuple(version, result);
}

KeySafe MessageDecryptor::parseKeySafe(const std::string &keySafeStr) const
{
    KeySafe result;
    Json::Value json;
    FWD_NESTED(json = Util::JsonHelper::readObject(keySafeStr),
               Util::JsonException,
               InvalidContentFormat("keySafe invalid"));

    result.msgFormat = Util::CheckedConverter::toUint32(json["msgFormat"]);
    if (result.msgFormat != 1)
    {
        throw UnsupportedContentVersion(std::to_string(result.msgFormat));
    }

    result.symmCipher = Util::CheckedConverter::toString(json["symmCipher"]);
    if (result.symmCipher != "AES-256/GCM")
    {
        throw InvalidContentFormat("MessageDecryptor::parseKeySafe: bad symmCipher");
    }

    result.hashAlgo = Util::CheckedConverter::toString(json["hashAlgo"]);
    if (result.hashAlgo != "SHA-512")
    {
        throw InvalidContentFormat("MessageDecryptor::parseKeySafe: bad hashAlgo");
    }

    std::vector<unsigned char> symmKey(
                Util::CheckedConverter::toVector(json["symmKey"]));
    result.symmKey = Crypto::SymmetricKeyLoader::fromVector(symmKey);
    if (result.symmKey.bitSize()
            != Crypto::SymmetricKeyGenerator::MESSAGE_SYMM_KEY_BITS)
    {
        throw InvalidContentFormat("MessageDecryptor::parseKeySafe: bad symmKey");
    }

    return result;
}

void MessageDecryptor::checkSize() const
{
    if (static_cast<size_t>(message_.keySafe.size())
            > Util::MESSAGE_KEY_SAFE_MAX_BYTES)
    {
        throw InvalidContentFormat(
                    std::string("keySafe too large: ") +
                    std::to_string(message_.keySafe.size()));
    }

    if (static_cast<size_t>(message_.content.size())
            > Util::MESSAGE_CONTENT_MAX_BYTES)
    {
        throw InvalidContentFormat(
                    std::string("content too large: ") +
                    std::to_string(message_.content.size()));
    }

    if (static_cast<size_t>(message_.meta.size())
            > Util::MESSAGE_META_MAX_BYTES)
    {
        throw InvalidContentFormat(
                    std::string("meta too large: ") +
                    std::to_string(message_.meta.size()));
    }
}

}
}
