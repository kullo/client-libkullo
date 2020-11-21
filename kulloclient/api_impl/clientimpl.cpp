/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/clientimpl.h"

#include <atomic>
#include <smartsqlite/logging.h>
#include <smartsqlite/sqlite3.h>

#include "kulloclient/api_impl/Address.h"
#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/clientaddressexistsworker.h"
#include "kulloclient/api_impl/clientcheckcredentialsworker.h"
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

    (errcode == SQLITE_WARNING_AUTOINDEX ? Log.d() : Log.w())
            << "[SQLite] " << SmartSqlite::errcodeToString(errcode)
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

nn_shared_ptr<Api::AsyncTask> ClientImpl::createSessionAsync(
        const Api::Address &address,
        const Api::MasterKey &masterKey,
        const std::string &dbFilePath,
        const nn_shared_ptr<Api::SessionListener> &sessionListener,
        const nn_shared_ptr<Api::ClientCreateSessionListener> &listener)
{
    kulloAssert(!dbFilePath.empty());

    const auto mainThread = std::this_thread::get_id();

    return nn_make_shared<AsyncTaskImpl>(
        std::make_shared<ClientCreateSessionWorker>(
                    address, masterKey, dbFilePath, sessionListener, listener, mainThread));
}

nn_shared_ptr<Api::AsyncTask> ClientImpl::addressExistsAsync(
        const Api::Address &address,
        const nn_shared_ptr<Api::ClientAddressExistsListener> &listener)
{
    return nn_make_shared<AsyncTaskImpl>(
        std::make_shared<ClientAddressExistsWorker>(address, listener));
}

nn_shared_ptr<Api::AsyncTask> ClientImpl::checkCredentialsAsync(
        const Api::Address &address,
        const Api::MasterKey &masterKey,
        const nn_shared_ptr<Api::ClientCheckCredentialsListener> &listener)
{
    return nn_make_shared<AsyncTaskImpl>(
                std::make_shared<ClientCheckCredentialsWorker>(address, masterKey, listener));
}

nn_shared_ptr<Api::AsyncTask> ClientImpl::generateKeysAsync(
        const nn_shared_ptr<Api::ClientGenerateKeysListener> &listener)
{
    return nn_make_shared<AsyncTaskImpl>(
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
