/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/Challenge.h"
#include "kulloclient/protocol/httpstructs.h"

namespace Kullo {
namespace ApiImpl {

class ChallengeImpl : public Api::Challenge
{
public:
    ChallengeImpl(const Protocol::Challenge &httpChallenge);

    Api::ChallengeType type() override;
    std::string text() override;

    Protocol::Challenge httpChallenge() const;

private:
    Protocol::Challenge httpChallenge_;
    Api::ChallengeType type_;
};

}
}
