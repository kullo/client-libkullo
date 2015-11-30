/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/clientcreatesessionworker.h"

#if defined __ANDROID__
    #include <stdlib.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "kulloclient/api_impl/sessionimpl.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/scopedbenchmark.h"

namespace fs = boost::filesystem;

namespace Kullo {
namespace ApiImpl {

ClientCreateSessionWorker::ClientCreateSessionWorker(std::shared_ptr<UserSettingsImpl> settings,
        const std::string &dbFilePath,
        std::shared_ptr<Api::SessionListener> sessionListener,
        std::shared_ptr<Api::ClientCreateSessionListener> listener)
    : settings_(settings)
    , dbFilePath_(dbFilePath)
    , sessionListener_(sessionListener)
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

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->finished(session);
        }
    }
    catch (fs::filesystem_error &ex)
    {
        Log.e() << "Couldn't create database directory: "
                << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(settings_->address(), Api::LocalError::Filesystem);
        }
    }
    catch (std::exception &ex)
    {
        Log.e() << "ClientCreateSessionWorker failed: " << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(settings_->address(), Api::LocalError::Unknown);
        }
    }
}

void ClientCreateSessionWorker::cancel()
{
    // work() cannot really be interrupted

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

std::shared_ptr<Api::Session> ClientCreateSessionWorker::makeSession() const
{
    // make DB session
    auto dbSession = Db::makeSession(dbFilePath_);
    Db::migrate(dbSession);
    kulloAssert(Db::hasCurrentSchema(dbSession));

    return std::make_shared<SessionImpl>(
                dbFilePath_, dbSession, settings_, sessionListener_);
}

}
}
