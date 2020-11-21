/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/clientcreatesessionworker.h"

#if defined __ANDROID__
    #include <stdlib.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "kulloclient/api/LocalError.h"
#include "kulloclient/api_impl/sessionimpl.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/multithreading.h"
#include "kulloclient/util/scopedbenchmark.h"

namespace fs = boost::filesystem;

namespace Kullo {
namespace ApiImpl {

ClientCreateSessionWorker::ClientCreateSessionWorker(
        const Api::Address &address,
        const Api::MasterKey &masterKey,
        const std::string &dbFilePath,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::ClientCreateSessionListener> listener,
        const boost::optional<std::thread::id> mainThread)
    : address_(address)
    , masterKey_(masterKey)
    , dbFilePath_(dbFilePath)
    , sessionListener_(sessionListener)
    , mainThread_(mainThread)
    , listener_(listener)
{}

void ClientCreateSessionWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("CreateSessionW");
    auto benchmark = Util::ScopedBenchmark("Creating session");
    K_RAII(benchmark);

#if defined __ANDROID__
    // Android has no global temp dir that is accessible by all apps, so use
    // the DB dir.
    auto dbDir = fs::path(dbFilePath_).remove_filename().string();
    if (setenv("SQLITE_TMPDIR", dbDir.c_str(), 1))
    {
        Log.e() << "Setting SQLITE_TMPDIR failed.";
    }
#endif

    try
    {
        auto session = makeSession();

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->finished(session);
        }
    }
    catch (std::exception &ex)
    {
        Log.e() << "ClientCreateSessionWorker failed: " << Util::formatException(ex);

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->error(address_, Api::LocalError::Unknown);
        }
    }
}

void ClientCreateSessionWorker::cancel()
{
    // work() cannot really be interrupted

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

nn_shared_ptr<Api::Session> ClientCreateSessionWorker::makeSession()
{
    auto credentials = Util::Credentials(
                std::make_shared<Util::KulloAddress>(address_.toString()),
                std::make_shared<Util::MasterKey>(masterKey_.blocks));
    auto sessionConfig = Db::SessionConfig(dbFilePath_, credentials);

    auto dbSession = Db::makeSession(sessionConfig);
    if (!Db::hasCurrentSchema(dbSession))
    {
        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->migrationStarted(address_);
        }
        Db::migrate(dbSession);
    }
    kulloAssert(Db::hasCurrentSchema(dbSession));

    return nn_make_shared<SessionImpl>(
                sessionConfig, dbSession, sessionListener_, mainThread_);
}

}
}
