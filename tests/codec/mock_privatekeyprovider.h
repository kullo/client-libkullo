/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
