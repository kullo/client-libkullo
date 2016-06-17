/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <stdexcept>
#include <vector>

#include <kulloclient/codec/exceptions.h>
#include <kulloclient/codec/messagedecryptor.h>
#include <kulloclient/crypto/asymmetriccryptor.h>
#include <kulloclient/crypto/asymmetrickeyloader.h>
#include <kulloclient/crypto/privatekey.h>
#include <kulloclient/crypto/privatekeyimpl.h>
#include <kulloclient/crypto/symmetriccryptor.h>
#include <kulloclient/crypto/symmetrickey.h>
#include <kulloclient/crypto/symmetrickeyloader.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/limits.h>
#include <kulloclient/util/misc.h>

#include "tests/codec/mock_messagedecryptor.h"
#include "tests/codec/mock_privatekeyprovider.h"
#include "tests/crypto/mock_asymmetriccryptor.h"
#include "tests/crypto/mock_asymmetrickeyloader.h"
#include "tests/crypto/mock_symmetriccryptor.h"
#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;

struct MetaTestData
{
    std::vector<unsigned char> ciphertext, encryptedMeta, plaintext;
    std::string meta;

    MetaTestData(metaVersion_type version)
    {
        ciphertext = {23, 42, 5};
        std::vector<unsigned char> versionVector(4);
        versionVector[0] = (version >> 24) & 0xff;
        versionVector[1] = (version >> 16) & 0xff;
        versionVector[2] = (version >>  8) & 0xff;
        versionVector[3] =  version        & 0xff;
        encryptedMeta.insert(encryptedMeta.end(), versionVector.cbegin(), versionVector.cend());
        encryptedMeta.insert(encryptedMeta.end(), ciphertext.cbegin(), ciphertext.cend());

        meta = "unencrypted meta";
        plaintext.insert(plaintext.end(), meta.cbegin(), meta.cend());
    }
};

class MessageDecryptor : public DbTest
{
protected:
    MessageDecryptor()
    {
        asymmCryptor.reset(new MockAsymmetricCryptor());
        symmCryptor.reset(new MockSymmetricCryptor());

        msg.dateReceived = Util::DateTime::nowUtc();
    }

    std::shared_ptr<MockPrivateKeyProvider> makePrivKeyProvider()
    {
        return std::make_shared<MockPrivateKeyProvider>(dbPath_);
    }

    std::unique_ptr<Codec::MessageDecryptor> makeUut(
            std::shared_ptr<Codec::PrivateKeyProvider> privKeyProvider = nullptr)
    {
        if (!privKeyProvider) privKeyProvider = makePrivKeyProvider();
        return make_unique<Codec::MessageDecryptor>(
                    msg,
                    privKeyProvider,
                    privateDataKey,
                    asymmCryptor.release(),
                    symmCryptor.release()
        );
    }

    Protocol::Message msg;
    Crypto::SymmetricKey privateDataKey;
    std::unique_ptr<MockAsymmetricCryptor> asymmCryptor;
    std::unique_ptr<MockSymmetricCryptor> symmCryptor;
};

K_TEST_F(MessageDecryptor, decryptsValidMessage)
{
    MockMessageDecryptor uut(
                msg, makePrivKeyProvider(), privateDataKey);
    EXPECT_CALL(uut, decryptKeySafe())
            .WillRepeatedly(
                Return(
                    std::string("{")
                    + R"("msgFormat": 1, "symmCipher": "AES-256/GCM", )"
                    + R"("hashAlgo": "SHA-512", )"
                    + R"("symmKey": "MDEyMzQ1Njc4OWFiY2RlZjAxMjM0NTY3ODlhYmNkZWY=")"
                    + "}"));
    EXPECT_CALL(uut, decryptContent(_))
            .WillRepeatedly(Return(Kullo::Codec::DecryptedContent()));
    EXPECT_CALL(uut, decryptMeta())
            .WillRepeatedly(
                Return(std::tuple<Kullo::metaVersion_type, std::string>()));

    auto decryptedMsg = uut.decrypt();
    EXPECT_THAT(decryptedMsg.id, Eq(msg.id));
    EXPECT_THAT(decryptedMsg.lastModified, Eq(msg.lastModified));
    EXPECT_THAT(decryptedMsg.dateReceived, Eq(msg.dateReceived));
}

K_TEST_F(MessageDecryptor, decryptThrowsOnLargeKeySafe)
{
    msg.keySafe.resize(Util::MESSAGE_KEY_SAFE_MAX_BYTES + 1);
    auto uut = makeUut();
    EXPECT_THROW(uut->decrypt(), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecryptor, decryptThrowsOnLargeContent)
{
    msg.content.resize(Util::MESSAGE_CONTENT_MAX_BYTES + 1);
    auto uut = makeUut();
    EXPECT_THROW(uut->decrypt(), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecryptor, decryptKeySafeThrowsOnEmptyKeySafe)
{
    auto uut = makeUut();
    EXPECT_THROW(uut->decryptKeySafe(), Kullo::Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecryptor, decryptKeySafeThrowsOnShortKeySafe)
{
    msg.keySafe = Util::to_vector("abc");
    auto uut = makeUut();
    EXPECT_THROW(uut->decryptKeySafe(), Kullo::Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecryptor, decryptKeySafeDecryptsCorrectly)
{
    msg.keySafe = Util::to_vector("\x12\x34\x56\x78\xbe\xef");
    std::vector<unsigned char> ciphertext(msg.keySafe.begin() + 4, msg.keySafe.end());

    std::string plaintext = "foo";
    std::vector<unsigned char> plaintextVec(plaintext.begin(), plaintext.end());
    Crypto::PrivateKey privkey(Crypto::AsymmetricKeyType::Encryption, nullptr);

    // test initialization
    EXPECT_THAT(ciphertext.size(), Eq(size_t{2}));
    EXPECT_THAT(ciphertext.at(0), 190);
    EXPECT_THAT(ciphertext.at(1), 239);

    Crypto::PrivateKey nullPrivKey(Crypto::AsymmetricKeyType::Encryption, nullptr);
    auto privKeyProvider = makePrivKeyProvider();
    EXPECT_CALL(*privKeyProvider, getKey(_, _)).WillRepeatedly(Return(nullPrivKey));
    EXPECT_CALL(*asymmCryptor, decrypt(ciphertext, privkey))
            .Times(1)
            .WillOnce(Return(plaintextVec));

    auto uut = makeUut(privKeyProvider);
    EXPECT_THAT(uut->decryptKeySafe(), Eq(plaintext));
}

K_TEST_F(MessageDecryptor, decryptContentThrowsOnEmptyPlaintext)
{
    EXPECT_CALL(*symmCryptor, decrypt(std::vector<unsigned char>(), _, A<const std::vector<unsigned char>&>()))
            .Times(1)
            .WillOnce(Return(std::vector<unsigned char>()));

    Codec::KeySafe keySafe;
    auto uut = makeUut();
    EXPECT_THROW(uut->decryptContent(keySafe), Kullo::Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecryptor, decryptContentThrowsOnShortPlaintext)
{
    std::string decryptResult = "1234567";
    EXPECT_CALL(*symmCryptor, decrypt(std::vector<unsigned char>(), _, A<const std::vector<unsigned char>&>()))
            .Times(1)
            .WillOnce(Return(std::vector<unsigned char>(
                                 decryptResult.cbegin(),
                                 decryptResult.cend())));

    Codec::KeySafe keySafe;
    auto uut = makeUut();
    EXPECT_THROW(uut->decryptContent(keySafe), Kullo::Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecryptor, decryptContentDecryptsLongEnoughInput)
{
    std::string sigKeyId("\x12\x34\x56\x78");
    std::string sigLen("\x00\x00\x00\x03", 4);
    std::string signature("\xfe\xdc\xba");
    std::string data("\x98\x76");

    std::vector<unsigned char> signatureVec(signature.cbegin(), signature.cend());

    std::vector<unsigned char> decryptResult;
    decryptResult.insert(decryptResult.end(), sigKeyId.cbegin(), sigKeyId.cend());
    decryptResult.insert(decryptResult.end(), sigLen.cbegin(), sigLen.cend());
    decryptResult.insert(decryptResult.end(), signature.cbegin(), signature.cend());
    decryptResult.insert(decryptResult.end(), data.cbegin(), data.cend());

    Codec::KeySafe keySafe;
    EXPECT_CALL(*symmCryptor, decrypt(std::vector<unsigned char>(), _, A<const std::vector<unsigned char>&>()))
            .Times(1)
            .WillOnce(Return(decryptResult));

    auto uut = makeUut();
    auto content = uut->decryptContent(keySafe);
    EXPECT_THAT(content.sigKeyId, Eq(id_type{305419896}));
    EXPECT_THAT(content.signature, Eq(signatureVec));
    EXPECT_THAT(std::string(content.data.begin(), content.data.end()), Eq(data));
}

K_TEST_F(MessageDecryptor, decryptMetaPassesThroughEmptyMeta)
{
    auto uut = makeUut();
    EXPECT_THAT(uut->decryptMeta(),
                Eq(std::make_tuple(Kullo::Codec::LATEST_META_VERSION, std::string())));
}

K_TEST_F(MessageDecryptor, decryptMeta0DecryptsNonEmptyInput)
{
    std::uint32_t version = 0;
    MetaTestData data(version);
    msg.meta = data.encryptedMeta;

    EXPECT_CALL(*symmCryptor, decrypt(data.ciphertext,
                                      privateDataKey,
                                      A<const std::vector<unsigned char>&>()))
            .Times(1)
            .WillOnce(Return(data.plaintext));

    auto uut = makeUut();
    auto meta = uut->decryptMeta();
    EXPECT_THAT(std::get<0>(meta), Eq(version));
    EXPECT_THAT(std::get<1>(meta), Eq(data.meta));
}

K_TEST_F(MessageDecryptor, decryptMeta1DecryptsNonEmptyInput)
{
    std::uint32_t version = 1;
    MetaTestData data(version);
    msg.meta = data.encryptedMeta;

    EXPECT_CALL(*symmCryptor, decrypt(data.ciphertext,
                                      privateDataKey,
                                      A<size_t>()))
            .Times(1)
            .WillOnce(Return(data.plaintext));

    auto uut = makeUut();
    auto meta = uut->decryptMeta();
    EXPECT_THAT(std::get<0>(meta), Eq(version));
    EXPECT_THAT(std::get<1>(meta), Eq(data.meta));
}

K_TEST_F(MessageDecryptor, decryptMeta2ReturnsOnlyVersion)
{
    std::uint32_t version = 2;
    MetaTestData data(version);
    msg.meta = data.encryptedMeta;

    auto uut = makeUut();
    auto meta = uut->decryptMeta();
    EXPECT_THAT(std::get<0>(meta), Eq(version));
    EXPECT_THAT(std::get<1>(meta), IsEmpty());
}
