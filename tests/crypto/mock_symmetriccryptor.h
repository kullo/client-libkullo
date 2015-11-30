/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/crypto/symmetriccryptor.h>

class MockSymmetricCryptor : public Kullo::Crypto::SymmetricCryptor
{
public:
    MOCK_CONST_METHOD3(decrypt,
                       std::vector<unsigned char>(
                           const std::vector<unsigned char> &ivAndCiphertext,
                           const Kullo::Crypto::SymmetricKey &key,
                           size_t ivLength
                       )
    );

    MOCK_CONST_METHOD3(decrypt,
                       std::vector<unsigned char>(
                           const std::vector<unsigned char> &ciphertext,
                           const Kullo::Crypto::SymmetricKey &key,
                           const std::vector<unsigned char> &iv
                       )
    );
};
