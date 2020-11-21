/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/Address.h"

#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Api {

Address::Address(const Util::KulloAddress &kulloAddress)
    : AddressBase(kulloAddress.username(), kulloAddress.domain())
{
}

Address::Address(const std::string &localPart, const std::string &domainPart)
    : Address(Util::KulloAddress(localPart + "#" + domainPart))
{
}

Address::Address(const std::string &addressAsString)
    : Address(Util::KulloAddress(addressAsString))
{
}

std::string Address::toString() const
{
    return localPart + "#" + domainPart;
}

bool Address::operator==(const Address &rhs) const
{
    return localPart == rhs.localPart && domainPart == rhs.domainPart;
}

std::ostream &operator<<(std::ostream &lhs, const Address &rhs)
{
    lhs << rhs.toString();
    return lhs;
}

}
}
