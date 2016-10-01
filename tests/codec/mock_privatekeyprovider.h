/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/codec/privatekeyprovider.h>

class MockPrivateKeyProvider : public Kullo::Codec::PrivateKeyProvider
{
public:
    MockPrivateKeyProvider(const Kullo::Db::SessionConfig &sessionConfig)
        : PrivateKeyProvider(sessionConfig)
    {
    }

    MOCK_METHOD2(
            getKey,
            Kullo::Crypto::PrivateKey(
                Kullo::Crypto::AsymmetricKeyType type,
                Kullo::id_type keyId)
            );
};
