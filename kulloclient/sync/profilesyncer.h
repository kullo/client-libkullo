/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <boost/optional.hpp>

#include "kulloclient/kulloclient-forwards.h"
#include "kulloclient/types.h"
#include "kulloclient/crypto/symmetrickey.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/protocol/profileclient.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/usersettings.h"

namespace Kullo {
namespace Sync {

/**
 * @brief Syncer for profile info
 */
class ProfileSyncer
{
public:
    struct {
        /**
         * @brief Emitted when a profile entry has been changed by the syncer.
         * @param key The profile entry's key
         */
        std::function<void(const std::string &key)>
        profileModified;
    } events;

    /**
     * @brief Creates a new profile info syncer.
     *
     * This doesn't automatically start the syncer. Call run() to start syncing.
     */
    explicit ProfileSyncer(
            const Util::Credentials &credentials,
            const Db::SharedSessionPtr &session,
            const std::shared_ptr<Http::HttpClient> &httpClient);

    ~ProfileSyncer();

    /**
     * @brief Start the syncer.
     */
    void run(std::shared_ptr<std::atomic<bool>> shouldCancel);

private:
    lastModified_type latestSync() const;
    void updateLatestSync(lastModified_type latestSync) const;
    Crypto::SymmetricKey &privateDataKey();
    std::string encrypt(const std::string &plaintext);
    std::string decrypt(const std::string &ciphertext);

    std::shared_ptr<std::atomic<bool>> shouldCancel_;
    std::unique_ptr<Protocol::ProfileClient> client_;
    Db::SharedSessionPtr session_;
    Util::Credentials credentials_;
    std::unique_ptr<Crypto::SymmetricKey> privateDataKey_;

    K_DISABLE_COPY(ProfileSyncer);
};

}
}
