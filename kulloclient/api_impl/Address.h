/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <iostream>

#include <boost/operators.hpp>

#include "kulloclient/api/Address_base.h"
#include "kulloclient/kulloclient-forwards.h"

namespace Kullo {
namespace Api {

struct Address final
        : public AddressBase
        , boost::equality_comparable<Address>
{
    // safe constructror
    Address(const Util::KulloAddress &kulloAddress);
    // required by djinni, thows if parts are invalid
    Address(const std::string &localPart, const std::string &domainPart);
    // convenience constructor, thows if address is invalid
    Address(const std::string &addressAsString);

    std::string toString() const;

    bool operator==(const Address &rhs) const;
    friend std::ostream &operator<<(std::ostream &lhs, const Address &rhs);
};

}
}

namespace std {
template <>
struct hash<Kullo::Api::Address> {
    size_t operator()(Kullo::Api::Address address) const {
        return std::hash<decltype(address.localPart)>()(address.localPart) ^
               std::hash<decltype(address.domainPart)>()(address.domainPart);
    }
};
}
