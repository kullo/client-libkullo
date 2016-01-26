/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/api/Address.h>

class MockAddress : public Kullo::Api::Address
{
public:
    MOCK_METHOD0(toString, std::string(void));
    MOCK_METHOD0(localPart, std::string(void));
    MOCK_METHOD0(domainPart, std::string(void));
    MOCK_METHOD1(isEqualTo, bool(const std::shared_ptr<Kullo::Api::Address>&));
    MOCK_METHOD1(isLessThan, bool(const std::shared_ptr<Kullo::Api::Address>&));
};
