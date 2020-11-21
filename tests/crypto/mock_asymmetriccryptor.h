/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/crypto/asymmetrickeyloader.h>

class MockAsymmetricCryptor : public Kullo::Crypto::AsymmetricCryptor
{
public:
    MOCK_CONST_METHOD2(decrypt,
                       std::vector<unsigned char>(
                           const std::vector<unsigned char> &ciphertext,
                           const Kullo::Crypto::PrivateKey &key
                       )
    );
};
