/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/api/MasterKey.h>

class MockMasterKey : public Kullo::Api::MasterKey
{
public:
    MOCK_METHOD0(pem, std::string(void));
    MOCK_METHOD0(dataBlocks, std::vector<std::string>(void));
    MOCK_METHOD1(isEqualTo, bool(const std::shared_ptr<Kullo::Api::MasterKey>&));
};
