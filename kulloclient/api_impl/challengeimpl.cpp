/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/challengeimpl.h"

#include "kulloclient/api/ChallengeType.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

ChallengeImpl::ChallengeImpl(const Protocol::Challenge &httpChallenge)
    : httpChallenge_(httpChallenge)
{
    if (httpChallenge_.type == "code")
    {
        type_ = Api::ChallengeType::Code;
    }
    else if (httpChallenge_.type == "reservation")
    {
        type_ = Api::ChallengeType::Reservation;
    }
    else if (httpChallenge_.type == "reset")
    {
        type_ = Api::ChallengeType::Reset;
    }
    else if (httpChallenge_.type == "blocked")
    {
        // this will lead to addressNotAvailable()
        throw Protocol::AddressBlocked();
    }
    else
    {
        throw Protocol::ProtocolError(
                    std::string("Unknown challenge type: ") +
                    httpChallenge_.type);
    }
}

Api::ChallengeType ChallengeImpl::type()
{
    return type_;
}

std::string ChallengeImpl::text()
{
    return httpChallenge_.text;
}

Protocol::Challenge ChallengeImpl::httpChallenge() const
{
    return httpChallenge_;
}

}
}
