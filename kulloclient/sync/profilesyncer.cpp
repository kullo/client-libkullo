/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/profilesyncer.h"

#include <algorithm>

#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/dao/symmetrickeydao.h"
#include "kulloclient/dao/syncdao.h"
#include "kulloclient/dao/usersettingsdao.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/base64.h"
#include "kulloclient/util/binary.h"
#include "kulloclient/util/events.h"

using namespace Kullo::Dao;
using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

ProfileSyncer::ProfileSyncer(
        const Credentials &credentials, const Db::SharedSessionPtr &session)
    : client_(make_unique<Protocol::ProfileClient>(
                  *credentials.address, *credentials.masterKey))
    , session_(session)
    , credentials_(credentials)
{
    kulloAssert(session_);
}

ProfileSyncer::~ProfileSyncer()
{
}

void ProfileSyncer::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    kulloAssert(shouldCancel);
    shouldCancel_ = shouldCancel;

    auto dao = UserSettingsDao(session_);

    // download remote changes
    const auto initialLatestSync = latestSync();
    auto maxLastModified = initialLatestSync;
    auto remoteChanges = client_->downloadChanges(initialLatestSync);
    for (auto remoteChange : remoteChanges)
    {
        remoteChange.value = decrypt(remoteChange.value);
        dao.setRemoteValue(remoteChange);
        EMIT(events.profileModified, remoteChange.key);
        maxLastModified = std::max(maxLastModified, remoteChange.lastModified);
    }
    if (maxLastModified > initialLatestSync) updateLatestSync(maxLastModified);

    // upload remaining local changes
    auto localChanges = dao.localChanges();
    for (auto localChange : localChanges)
    {
        try
        {
            localChange.value = encrypt(localChange.value);
            auto remoteChange = client_->uploadChange(localChange);
            remoteChange.value = decrypt(remoteChange.value);
            dao.setRemoteValue(remoteChange);
        }
        catch (Protocol::Conflict)
        {
            // new version will be downloaded on next sync
            continue;
        }
    }
}

lastModified_type ProfileSyncer::latestSync() const
{
    return SyncDao::timestamp(Dao::SyncDao::DataType::Profile, session_);
}

void ProfileSyncer::updateLatestSync(lastModified_type latestSync) const
{
    SyncDao::setTimestamp(Dao::SyncDao::DataType::Profile, latestSync, session_);
}

Crypto::SymmetricKey &ProfileSyncer::privateDataKey()
{
    if (!privateDataKey_)
    {
        privateDataKey_ = make_unique<Crypto::SymmetricKey>(
                    Dao::SymmetricKeyDao::loadKey(
                        Dao::SymmetricKeyDao::PRIVATE_DATA_KEY,
                        session_));
    }
    return *privateDataKey_;
}

std::string ProfileSyncer::encrypt(const std::string &plaintext)
{
    auto iv = Crypto::SymmetricKeyGenerator().makeRandomIV(
                Crypto::SymmetricCryptor::RANDOM_IV_BYTES);
    return Util::Base64::encode(
                Crypto::SymmetricCryptor().encrypt(
                    Util::to_vector(plaintext),
                    privateDataKey(),
                    iv,
                    Crypto::PrependIV::True));
}

std::string ProfileSyncer::decrypt(const std::string &ciphertext)
{
    return Util::to_string(
                Crypto::SymmetricCryptor().decrypt(
                    Util::Base64::decode(ciphertext),
                    privateDataKey(),
                    Crypto::SymmetricCryptor::RANDOM_IV_BYTES));
}

}
}
