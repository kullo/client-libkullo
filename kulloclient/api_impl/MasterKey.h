/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
