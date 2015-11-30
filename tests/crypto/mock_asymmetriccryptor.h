/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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
