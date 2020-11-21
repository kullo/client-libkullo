/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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

    PublicKeysClient(const std::shared_ptr<Http::HttpClient> &httpClient);

    Kullo::Protocol::KeyPair getPublicKey(
            const Util::KulloAddress &username,
            id_type id);

private:
    static KeyPair parseJsonKeyPair(const Json::Value &jsonObject);
};

}
}
