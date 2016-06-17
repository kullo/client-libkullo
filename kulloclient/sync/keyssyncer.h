/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/http/HttpClient.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace Sync {

class KeysSyncer
{
public:
    explicit KeysSyncer(
            const Util::Credentials &credentials,
            const Db::SharedSessionPtr &session,
            const std::shared_ptr<Http::HttpClient> &httpClient);
    ~KeysSyncer();

    /**
     * @brief Start the syncer.
     */
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);


private:
    void storePrivateDataKey(const Protocol::SymmetricKeys &symmKeys);
    void storeAsymmKeyPairs(const std::vector<Protocol::KeyPair> &keyPairs);

    Db::SharedSessionPtr session_;
    std::unique_ptr<Protocol::KeysClient> client_;
    Util::Credentials credentials_;

    K_DISABLE_COPY(KeysSyncer);
};

}
}
