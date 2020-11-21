/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <boost/operators.hpp>

#include "kulloclient/api/MasterKey_base.h"
#include "kulloclient/kulloclient-forwards.h"

namespace Kullo {
namespace Api {

struct MasterKey
        : public MasterKeyBase
        , boost::equality_comparable<MasterKey>
{
    // safe constructor
    MasterKey(const Util::MasterKey &masterKey);
    // requied by djinni, throws if blocks are invalid
    MasterKey(const std::vector<std::string> &blocks);

    bool operator==(const MasterKey &rhs) const;
};

}
}
