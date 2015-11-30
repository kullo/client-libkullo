/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/db/dbsession.h>

#include "tests/kullotest.h"
#include "tests/testutil.h"

class DbTest : public KulloTest
{
public:
    DbTest()
        : dbPath_(TestUtil::tempDbFileName())
        , session_(Kullo::Db::makeSession(dbPath_))
    {
        #ifdef KULLO_DB_TRACING
        session->setTracingCallback(&tracingCallback);
        #endif
        #ifdef KULLO_DB_PROFILING
        session->setProfilingCallback(&profilingCallback);
        #endif

        Kullo::Db::migrate(session_);
    }

    ~DbTest() override
    {
        if (session_.use_count() > 1)
        {
            std::cerr << "WARNING: ~DbTest called, but the session is still in use elsewhere. "
                      << "This indicates a memory leak. "
                      << "(Session use count: " << session_.use_count() << ")" << std::endl;
        }
    }

protected:
    std::string dbPath_;
    Kullo::Db::SharedSessionPtr session_;

private:
    #ifdef KULLO_DB_TRACING
    static void tracingCallback(void *, const char *sql)
    {
        std::cerr << "[SQL] " << sql << std::endl;
    }
    #endif

    #ifdef KULLO_DB_PROFILING
    static void profilingCallback(void *, const char *sql, std::uint64_t nsecs)
    {
        std::cerr << "[SQL] " << nsecs / 1000000 << "ms, " << sql << std::endl;
    }
    #endif
};
