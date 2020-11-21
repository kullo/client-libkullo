/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/sync/keyssyncer.h"

#include "kulloclient/codec/exceptions.h"
#include "kulloclient/crypto/symmetriccryptor.h"
#include "kulloclient/crypto/symmetrickeyloader.h"
#include "kulloclient/dao/asymmetrickeypairdao.h"
#include "kulloclient/dao/symmetrickeydao.h"
#include "kulloclient/protocol/keysclient.h"
#include "kulloclient/sync/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/datetime.h"
#include "kulloclient/util/kulloaddress.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Sync {

KeysSyncer::KeysSyncer(
        const Credentials &credentials,
        const Db::SharedSessionPtr &session,
        const std::shared_ptr<Http::HttpClient> &httpClient)
    : session_(session),
      credentials_(credentials)
{
    client_.reset(new Protocol::KeysClient(
                     *credentials_.address,
                     *credentials_.masterKey,
                      httpClient));
}

KeysSyncer::~KeysSyncer()
{
}

void KeysSyncer::run(std::shared_ptr<std::atomic<bool>> shouldCancel)
{
    auto symmKeys = client_->getSymmKeys();
    storePrivateDataKey(symmKeys);

    if (*shouldCancel) throw SyncCanceled();

    auto keyPairs = client_->getAsymmKeyPairs();
    storeAsymmKeyPairs(keyPairs);
}

void KeysSyncer::storePrivateDataKey(const Protocol::SymmetricKeys &symmKeys)
{
    // decrypt
    Crypto::SymmetricCryptor cryptor;
    auto privateDataKey = cryptor.decrypt(
                symmKeys.privateDataKey,
                Crypto::SymmetricKeyLoader::fromMasterKey(*credentials_.masterKey),
                cryptor.RANDOM_IV_BYTES);

    // save
    auto dao = Dao::SymmetricKeyDao::load(Dao::SymmetricKeyDao::PRIVATE_DATA_KEY,
                                          session_);
    if (!dao)
    {
        dao.reset(new Dao::SymmetricKeyDao(session_));
        dao->setKeyType(Dao::SymmetricKeyDao::PRIVATE_DATA_KEY);
    }
    dao->setKey(privateDataKey);
    dao->save();
}

void KeysSyncer::storeAsymmKeyPairs(const std::vector<Protocol::KeyPair> &keyPairs)
{
    for (const Protocol::KeyPair &keyPair : keyPairs)
    {
        auto type = Crypto::AsymmetricKeyType::Invalid;
        FWD_NESTED(type = Crypto::keyTypeFromString(keyPair.type),
                   InvalidArgument,
                   Codec::InvalidContentFormat("unknown keyPair.type"));

        // don't store keys when there's no private key, since we couldn't use
        // them for signing
        if (keyPair.privkey.empty()) continue;

        auto dao = Dao::AsymmetricKeyPairDao::load(type, keyPair.id, session_);
        if (!dao)
        {
            dao.reset(new Dao::AsymmetricKeyPairDao(session_));
            dao->setId(keyPair.id);
            dao->setKeyType(type);

            auto privateDataKey = Dao::SymmetricKeyDao::loadKey(
                        Dao::SymmetricKeyDao::PRIVATE_DATA_KEY,
                        session_);

            Crypto::SymmetricCryptor cryptor;
            auto privkey = cryptor.decrypt(
                        keyPair.privkey,
                        privateDataKey,
                        cryptor.RANDOM_IV_BYTES);

            dao->setPubkey(keyPair.pubkey);
            dao->setPrivkey(privkey);
            dao->setValidFrom(keyPair.validFrom.toString());
            dao->setValidUntil(keyPair.validUntil.toString());
        }
        dao->setRevocation(keyPair.revocation);
        dao->save();
    }
}

}
}
