/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
