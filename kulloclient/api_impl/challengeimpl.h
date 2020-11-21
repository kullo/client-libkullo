/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
