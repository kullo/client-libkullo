/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>
#include <vector>

#include "kulloclient/types.h"
#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/util/datetime.h"

namespace Kullo {
namespace Codec {

const metaVersion_type LATEST_META_VERSION = 1;

struct KeySafe
{
    std::uint32_t msgFormat = 0;
    std::string symmCipher;
    Crypto::SymmetricKey symmKey;
    std::string hashAlgo;
};

struct DecryptedContent
{
    id_type sigKeyId = 0;
    std::vector<unsigned char> signature;
    std::vector<unsigned char> data;
};

struct DecryptedMessage final
{
    id_type id = 0;
    lastModified_type lastModified = 0;
    Util::DateTime dateReceived;

    KeySafe keySafe;
    DecryptedContent content;
    metaVersion_type metaVersion = 0;
    std::string meta;

    DecryptedMessage(Util::DateTime dateReceived)
        : dateReceived(std::move(dateReceived))
    {}
};

/// Message struct to hold an encoded but unencrypted message
struct EncodedMessage
{
    /// The message content (unencrypted JSON)
    std::vector<unsigned char> content;

    /// Attachment data (unencrypted blob)
    std::vector<unsigned char> attachments;
};

}
}
