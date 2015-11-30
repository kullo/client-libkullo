/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/protocol/baseclient.h"
#include "kulloclient/protocol/httpstructs.h"

namespace Kullo {
namespace Protocol {

class PublicKeysClient : public BaseClient
{
public:
    static const id_type LATEST_ENCRYPTION_PUBKEY;

    Kullo::Protocol::KeyPair getPublicKey(
            const Util::KulloAddress &username,
            id_type id);

private:
    static KeyPair parseJsonKeyPair(const Json::Value &jsonObject);
};

}
}
