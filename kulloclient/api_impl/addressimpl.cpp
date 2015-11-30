/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/addressimpl.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace ApiImpl {

AddressImpl::AddressImpl(const std::string &address)
    : address_(Util::KulloAddress(address))
{}

AddressImpl::AddressImpl(const Util::KulloAddress &address)
    : address_(address)
{}

std::string AddressImpl::toString()
{
    return address_.toString();
}

std::string AddressImpl::localPart()
{
    return address_.username();
}

std::string AddressImpl::domainPart()
{
    return address_.domain();
}

bool AddressImpl::isEqualTo(const std::shared_ptr<Api::Address> &other)
{
    auto otherAddressImpl = std::dynamic_pointer_cast<AddressImpl>(other);
    kulloAssert(otherAddressImpl);
    return address_ == otherAddressImpl->address_;
}

bool AddressImpl::isLessThan(const std::shared_ptr<Api::Address> &rhs)
{
    auto rhsAddressImpl = std::dynamic_pointer_cast<AddressImpl>(rhs);
    kulloAssert(rhsAddressImpl);
    return address_ < rhsAddressImpl->address_;
}

Util::KulloAddress AddressImpl::address() const
{
    return address_;
}

}
}
