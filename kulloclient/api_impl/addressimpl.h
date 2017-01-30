/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/Address.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace ApiImpl {

class AddressImpl : public Api::Address
{
public:
    explicit AddressImpl(const std::string &address);
    explicit AddressImpl(const Util::KulloAddress &address);

    std::string toString() override;
    std::string localPart() override;
    std::string domainPart() override;
    bool isEqualTo(const std::shared_ptr<Address> &other) override;
    bool isLessThan(const std::shared_ptr<Address> &rhs) override;

    Util::KulloAddress address() const;

private:
    Util::KulloAddress address_;
};

}
}
