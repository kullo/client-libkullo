/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/messageencryptor.h"

#include <jsoncpp/jsoncpp.h>

#include "kulloclient/crypto/asymmetriccryptor.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/signer.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/util/binary.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Codec {

Protocol::SendableMessage MessageEncryptor::makeSendableMessage(
        const EncodedMessage &encodedMsg,
        id_type encKeyId,
        const Crypto::PublicKey &encKey,
        id_type sigKeyId,
        const Crypto::PrivateKey &sigKeyPair)
{
    makeKeySafe(encKeyId, encKey);
    encryptContent(encodedMsg.content, sigKeyId, sigKeyPair);
    encryptAttachments(encodedMsg);
    return sendableMessage_;
}

std::vector<unsigned char> MessageEncryptor::encryptMeta(
        const std::vector<unsigned char> &plaintext,
        const Crypto::SymmetricKey &key)
{
    static const std::vector<unsigned char> version = {0, 0, 0, 1};
    auto iv = Crypto::SymmetricKeyGenerator().makeRandomIV(
                Crypto::SymmetricCryptor::RANDOM_IV_BYTES);
    auto ciphertext = Crypto::SymmetricCryptor().encrypt(
                plaintext, key, iv, Crypto::PrependIV::True);
    ciphertext.insert(ciphertext.begin(), version.cbegin(), version.cend());
    return ciphertext;
}

void MessageEncryptor::makeKeySafe(
        id_type encKeyId,
        const Crypto::PublicKey &encKey)
{
    Crypto::SymmetricKeyGenerator keyGen;
    symmKey_ = keyGen.makeMessageSymmKey();

    Json::Value keySafeJson(Json::objectValue);
    keySafeJson["msgFormat"]  = 1;
    keySafeJson["symmCipher"] = "AES-256/GCM";
    keySafeJson["symmKey"]    = symmKey_.toBase64();
    keySafeJson["hashAlgo"]   = "SHA-512";

    Crypto::AsymmetricCryptor asymmCryptor;

    // encKeyId
    sendableMessage_.keySafe.push_back((encKeyId >> 24) & 0xff);
    sendableMessage_.keySafe.push_back((encKeyId >> 16) & 0xff);
    sendableMessage_.keySafe.push_back((encKeyId >> 8) & 0xff);
    sendableMessage_.keySafe.push_back(encKeyId & 0xff);

    auto encryptedKeySafe = asymmCryptor.encrypt(
                Util::to_vector(Json::FastWriter().write(keySafeJson)),
                encKey
    );

    std::copy(encryptedKeySafe.cbegin(), encryptedKeySafe.cend(),
              std::back_inserter(sendableMessage_.keySafe));
}

void MessageEncryptor::encryptContent(
        const std::vector<unsigned char> &content,
        id_type sigKeyId,
        const Crypto::PrivateKey &sigKeyPair)
{
    auto sig = Crypto::Signer().sign(content, sigKeyPair);
    std::vector<unsigned char> unencryptedContent;

    // sigKeyId
    unencryptedContent.push_back((sigKeyId >> 24) & 0xff);
    unencryptedContent.push_back((sigKeyId >> 16) & 0xff);
    unencryptedContent.push_back((sigKeyId >> 8) & 0xff);
    unencryptedContent.push_back(sigKeyId & 0xff);

    // signature size
    std::uint32_t sigSize = sig.size();
    unencryptedContent.push_back((sigSize >> 24) & 0xff);
    unencryptedContent.push_back((sigSize >> 16) & 0xff);
    unencryptedContent.push_back((sigSize >> 8) & 0xff);
    unencryptedContent.push_back(sigSize & 0xff);

    // signature
    std::copy(sig.cbegin(), sig.cend(), std::back_inserter(unencryptedContent));

    // content
    std::copy(content.cbegin(), content.cend(), std::back_inserter(unencryptedContent));

    sendableMessage_.content = Crypto::SymmetricCryptor().encrypt(
                unencryptedContent,
                symmKey_,
                Util::to_vector(Crypto::SymmetricCryptor::CONTENT_IV),
                Crypto::PrependIV::False
                );
}

void MessageEncryptor::encryptAttachments(const EncodedMessage &encodedMsg)
{
    if (!encodedMsg.attachments.empty())
    {
        sendableMessage_.attachments = Crypto::SymmetricCryptor().encrypt(
                    encodedMsg.attachments,
                    symmKey_,
                    Util::to_vector(Crypto::SymmetricCryptor::ATTACHMENTS_IV),
                    Crypto::PrependIV::False
                    );
    }
}

}
}
