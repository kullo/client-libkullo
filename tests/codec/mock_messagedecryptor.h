/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/codec/messagedecryptor.h>

class MockMessageDecryptor : public Kullo::Codec::MessageDecryptor
{
public:
    MockMessageDecryptor(
            const Kullo::Protocol::Message &message,
            std::shared_ptr<Kullo::Codec::PrivateKeyProvider> privKeyProvider,
            const Kullo::Crypto::SymmetricKey &privateDataKey)
        : MessageDecryptor(message, privKeyProvider, privateDataKey) {}

    MOCK_CONST_METHOD0(decryptKeySafe,
                       std::string()
    );
    MOCK_CONST_METHOD1(decryptContent,
                       Kullo::Codec::DecryptedContent(
                           const Kullo::Codec::KeySafe &keySafe
                       )
    );
    MOCK_CONST_METHOD0(decryptMeta,
                       std::tuple<Kullo::metaVersion_type, std::string>()
    );
};
