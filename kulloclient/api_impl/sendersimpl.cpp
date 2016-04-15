/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sendersimpl.h"

#include "kulloclient/api_impl/addressimpl.h"
#include "kulloclient/dao/avatardao.h"
#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

SendersImpl::SendersImpl(
        std::shared_ptr<SessionData> session,
        std::shared_ptr<Api::SessionListener> sessionListener)
    : sessionData_(session)
    , sessionListener_(sessionListener)
    , avatars_(Dao::AvatarDao::all(sessionData_->dbSession_))
{
    auto daos = Dao::ParticipantDao::all(sessionData_->dbSession_);
    while (const auto &dao = daos->next())
    {
        senders_.emplace_hint(senders_.end(), dao->messageId(), *dao);
    }
}

std::string SendersImpl::name(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = senders_.find(msgId);
    return (iter != senders_.end()) ? iter->second.name(): "";
}

std::shared_ptr<Api::Address> SendersImpl::address(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = senders_.find(msgId);
    return (iter != senders_.end())
            ? std::make_shared<AddressImpl>(iter->second.address())
            : nullptr;
}

std::string SendersImpl::organization(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = senders_.find(msgId);
    return (iter != senders_.end()) ? iter->second.organization(): "";
}

std::string SendersImpl::avatarMimeType(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = senders_.find(msgId);
    return (iter != senders_.end()) ? iter->second.avatarMimeType(): "";
}

std::vector<uint8_t> SendersImpl::avatar(int64_t msgId)
{
    kulloAssert(msgId >= Kullo::ID_MIN && msgId <= Kullo::ID_MAX);

    auto iter = senders_.find(msgId);
    if (iter == senders_.end()) return {};

    auto hash = iter->second.avatarHash();
    if (!hash) return {};

    auto avatarIter = avatars_.find(*hash);
    if (avatarIter == avatars_.end())
    {
        Log.w() << "Avatar with hash " << *hash << " not found.";
        return {};
    }
    return avatarIter->second;
}

Event::ApiEvents SendersImpl::senderAdded(int64_t msgId)
{
    auto dao = Dao::ParticipantDao::load(msgId, sessionData_->dbSession_);
    if (!dao) throw Db::DatabaseIntegrityError("SendersImpl::senderAdded");
    senders_.emplace(dao->messageId(), *dao);

    auto avatarHash = dao->avatarHash();
    if (avatarHash)
    {
        auto avatar = Dao::AvatarDao::load(*avatarHash, sessionData_->dbSession_);
        avatars_.emplace(*avatarHash, avatar);
    }

    return {{}};
}

Event::ApiEvents SendersImpl::messageRemoved(int64_t convId, int64_t msgId)
{
    K_UNUSED(convId);

    auto daoIter = senders_.find(msgId);
    if (daoIter != senders_.end())
    {
        daoIter->second.deletePermanently();
        senders_.erase(daoIter);
    }
    return {{}};
}

}
}
