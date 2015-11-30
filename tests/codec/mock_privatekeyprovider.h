/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/codec/privatekeyprovider.h>

class MockPrivateKeyProvider : public Kullo::Codec::PrivateKeyProvider
{
public:
    MockPrivateKeyProvider(const std::string &dbPath)
        : PrivateKeyProvider(dbPath)
    {
    }

    MOCK_METHOD2(
            getKey,
            Kullo::Crypto::PrivateKey(
                Kullo::Crypto::AsymmetricKeyType type,
                Kullo::id_type keyId)
            );
};
