/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/db/dbsession.h>

#include "tests/kullotest.h"

class DbTest : public KulloTest
{
public:
    DbTest();
    ~DbTest() override;

protected:
    Kullo::Db::SessionConfig sessionConfig_;
    Kullo::Db::SharedSessionPtr session_;
};
