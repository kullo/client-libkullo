/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
