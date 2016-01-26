/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "kulloclient/codec/codecstructs.h"
#include "kulloclient/codec/privatekeyprovider.h"
#include "kulloclient/crypto/asymmetriccryptor.h"
#include "kulloclient/crypto/asymmetrickeyloader.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/httpstructs.h"

namespace Kullo {
namespace Codec {

class MessageDecryptor
{
public:
    MessageDecryptor(
            const Protocol::Message &message,
            std::shared_ptr<PrivateKeyProvider> privKeyProvider,
            const Crypto::SymmetricKey &privateDataKey,
            Crypto::AsymmetricCryptor *asymmCryptor = nullptr,
            Crypto::SymmetricCryptor *symmCryptor = nullptr);

    virtual ~MessageDecryptor() = default;

    DecryptedMessage decrypt() const;
    virtual std::string decryptKeySafe() const;
    virtual DecryptedContent decryptContent(const KeySafe &keySafe) const;
    virtual std::tuple<metaVersion_type, std::string> decryptMeta() const;

private:
    KeySafe parseKeySafe(const std::string &keySafeStr) const;
    void checkSize() const;

    const Protocol::Message &message_;
    std::shared_ptr<PrivateKeyProvider> privKeyProvider_;
    const Crypto::SymmetricKey privateDataKey_;
    std::unique_ptr<Crypto::AsymmetricCryptor> asymmCryptor_;
    std::unique_ptr<Crypto::SymmetricCryptor> symmCryptor_;
};

}
}
