/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional.hpp>
#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/protocol/baseclient.h"
#include "kulloclient/protocol/httpstructs.h"
#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Protocol {

class AccountsClient : public BaseClient
{
public:
    boost::optional<Challenge> registerAccount(
            const Util::KulloAddress &address,
            const SymmetricKeys &symmKeys,
            const KeyPair &encKeys,
            const KeyPair &sigKeys,
            const boost::optional<Challenge> &challenge,
            const boost::optional<std::string> &challengeAnswer);

private:
    Json::Value makeRegistrationDocument(
            const Util::KulloAddress &address,
            const SymmetricKeys &symmKeys,
            const KeyPair &encKeys,
            const KeyPair &sigKeys,
            const boost::optional<Challenge> &challenge,
            const boost::optional<std::string> &challengeAnswer);

    static Challenge parseJsonChallenge(const Json::Value &contentJson);
};

}
}
