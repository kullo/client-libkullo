/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/dbtest.h"

#include <kulloclient/util/kulloaddress.h>
#include <kulloclient/util/masterkey.h>

#include "tests/testdata.h"
#include "tests/testutil.h"

namespace {

#ifdef KULLO_DB_TRACING
void tracingCallback(void *, const char *sql)
{
    std::cerr << "[SQL] " << sql << std::endl;
}
#endif

#ifdef KULLO_DB_PROFILING
void profilingCallback(void *, const char *sql, std::uint64_t nsecs)
{
    std::cerr << "[SQL] " << nsecs / 1000000 << "ms, " << sql << std::endl;
}
#endif

}

DbTest::DbTest()
    : sessionConfig_(
          TestUtil::tempDbFileName(),
          Kullo::Util::Credentials(
              std::make_shared<Kullo::Util::KulloAddress>("example#kullo.dev"),
              std::make_shared<Kullo::Util::MasterKey>(MasterKeyData::VALID_PEM)))
    , session_(Kullo::Db::makeSession(sessionConfig_))
{
#ifdef KULLO_DB_TRACING
    session->setTracingCallback(&tracingCallback);
#endif
#ifdef KULLO_DB_PROFILING
    session->setProfilingCallback(&profilingCallback);
#endif

    Kullo::Db::migrate(session_);
}

DbTest::~DbTest()
{
    if (session_.use_count() > 1)
    {
        std::cerr << "WARNING: ~DbTest called, but the session is still in use elsewhere. "
                  << "This indicates a memory leak. "
                  << "(Session use count: " << session_.use_count() << ")" << std::endl;
    }
}
