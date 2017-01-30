/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <vector>

#include "kulloclient/codec/messageencoder.h"
#include "kulloclient/crypto/privatekey.h"
#include "kulloclient/crypto/publickey.h"
#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/protocol/httpstructs.h"

namespace Kullo {
namespace Codec {

class MessageEncryptor
{
public:
    virtual ~MessageEncryptor() = default;

    virtual Protocol::SendableMessage makeSendableMessage(
            const EncodedMessage &encodedMsg,
            id_type encKeyId,
            const Crypto::PublicKey &encKey,
            id_type sigKeyId,
            const Crypto::PrivateKey &sigKeyPair);

    virtual std::vector<unsigned char> encryptMeta(
            const std::vector<unsigned char> &plaintext,
            const Crypto::SymmetricKey &key);

protected:
    struct State
    {
        Crypto::SymmetricKey symmKey_;
        std::vector<unsigned char> keySafe_;
        Protocol::SendableMessage sendableMessage_;
    };

    virtual void makeKeySafe(
            State &state,
            id_type encKeyId,
            const Crypto::PublicKey &encKey);

    virtual void encryptContent(
            State &state,
            const std::vector<unsigned char> &content,
            id_type sigKeyId,
            const Crypto::PrivateKey &sigKeyPair);

    virtual void encryptAttachments(
            State &state,
            const EncodedMessage &encodedMsg);
};

}
}
