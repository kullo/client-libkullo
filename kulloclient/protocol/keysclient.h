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

class KeysClient : public BaseClient
{
public:
    /**
     * @brief Creates a new KeysClient.
     * @param address Address of the local user
     */
    KeysClient(
            const Util::KulloAddress &address,
            const Util::MasterKey &masterKey,
            const std::shared_ptr<Http::HttpClient> &httpClient);

    Kullo::Protocol::SymmetricKeys getSymmKeys();
    void modifySymmKeys(const SymmetricKeys &symmKeys);

    std::vector<Kullo::Protocol::KeyPair> getAsymmKeyPairs();

private:
    static SymmetricKeys parseJsonSymmKeys(const Json::Value &jsonObject);
    static KeyPair parseJsonKeyPair(const Json::Value &jsonObject);
    static id_type parseJsonId(const Json::Value &jsonObject);
};

}
}
