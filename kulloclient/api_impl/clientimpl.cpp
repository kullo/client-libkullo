/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/clientimpl.h"

#include <atomic>
#include <smartsqlite/logging.h>

#include "kulloclient/api/Address.h"
#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/clientaddressexistsworker.h"
#include "kulloclient/api_impl/clientcheckloginworker.h"
#include "kulloclient/api_impl/clientcreatesessionworker.h"
#include "kulloclient/api_impl/clientgeneratekeysworker.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/version.h"

namespace Kullo {
namespace ApiImpl {

namespace {
std::atomic<bool> staticallyInitialized_ { false };
}

static void sqliteLogCallback(void *extraArg, int errcode, char *message)
{
    K_UNUSED(extraArg);
    Log.e() << "[SQLite] " << SmartSqlite::errcodeToString(errcode)
            << ": " << message;
}

ClientImpl::ClientImpl()
{
    // only do initialization if it hasn't been done previously
    if (!staticallyInitialized_.exchange(true))
    {
        SmartSqlite::setLogCallback(&sqliteLogCallback);
        Log.d() << "SQLite logging initialized";
    }
}

std::shared_ptr<Api::AsyncTask> ClientImpl::createSessionAsync(
        const std::shared_ptr<Api::UserSettings> &settings,
        const std::string &dbFilePath,
        const std::shared_ptr<Api::SessionListener> &sessionListener,
        const std::shared_ptr<Api::ClientCreateSessionListener> &listener)
{
    kulloAssert(settings);
    kulloAssert(!dbFilePath.empty());
    kulloAssert(sessionListener);
    kulloAssert(listener);

    auto userSettingsImpl =
            std::dynamic_pointer_cast<UserSettingsImpl>(settings);
    kulloAssert(userSettingsImpl);

    return std::make_shared<AsyncTaskImpl>(
        std::make_shared<ClientCreateSessionWorker>(
                    userSettingsImpl, dbFilePath, sessionListener, listener));
}

std::shared_ptr<Api::AsyncTask> ClientImpl::addressExistsAsync(
        const std::shared_ptr<Api::Address> &address,
        const std::shared_ptr<Api::ClientAddressExistsListener> &listener)
{
    kulloAssert(address);
    kulloAssert(listener);

    return std::make_shared<AsyncTaskImpl>(
        std::make_shared<ClientAddressExistsWorker>(address, listener));
}

std::shared_ptr<Api::AsyncTask> ClientImpl::checkLoginAsync(
        const std::shared_ptr<Api::Address> &address,
        const std::shared_ptr<Api::MasterKey> &masterKey,
        const std::shared_ptr<Api::ClientCheckLoginListener> &listener)
{
    kulloAssert(address);
    kulloAssert(masterKey);
    kulloAssert(listener);

    return std::make_shared<AsyncTaskImpl>(
                std::make_shared<ClientCheckLoginWorker>(address, masterKey, listener));
}

std::shared_ptr<Api::AsyncTask> ClientImpl::generateKeysAsync(
        const std::shared_ptr<Api::ClientGenerateKeysListener> &listener)
{
    kulloAssert(listener);

    return std::make_shared<AsyncTaskImpl>(
        std::make_shared<ClientGenerateKeysWorker>(listener));
}

std::unordered_map<std::string, std::string> ClientImpl::versions()
{
    std::unordered_map<std::string, std::string> result;
    result[Api::Client::BOTAN] = Util::Version::botanVersion();
    result[Api::Client::BOOST] = Util::Version::boostVersion();
    result[Api::Client::LIBKULLO] = Util::Version::libkulloVersion();
    result[Api::Client::JSONCPP] = Util::Version::jsoncppVersion();
    result[Api::Client::SMARTSQLITE] = Util::Version::smartSqliteVersion();
    result[Api::Client::SQLITE] = Util::Version::sqliteVersion();
    return result;
}

}
}
