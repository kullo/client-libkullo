/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sessionaccountinfoworker.h"

#include "kulloclient/api/AccountInfo.h"
#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/multithreading.h"
#include "kulloclient/util/numeric_cast.h"
#include "kulloclient/registry.h"

namespace Kullo {
namespace ApiImpl {

SessionAccountInfoWorker::SessionAccountInfoWorker(
        const Util::KulloAddress &address,
        const Util::MasterKey &masterKey,
        std::shared_ptr<Api::SessionAccountInfoListener> listener)
    : accountClient_(
          address,
          masterKey,
          Registry::httpClientFactory()->createHttpClient())
    , listener_(listener)
{}

void SessionAccountInfoWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AccountInfoW");

    try
    {
        const auto accountInfo = accountClient_.getInfo();

        // Convert optional<uint64> to optional<int64>
        boost::optional<int64_t> storageQuota;
        boost::optional<int64_t> storageUsed;
        if (accountInfo.storageQuota) storageQuota = Util::numeric_cast<int64_t>(*accountInfo.storageQuota);
        if (accountInfo.storageUsed) storageUsed = Util::numeric_cast<int64_t>(*accountInfo.storageUsed);

        const auto out = Api::AccountInfo(
                    accountInfo.planName,
                    storageQuota,
                    storageUsed,
                    accountInfo.settingsLocation);

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->finished(out);
        }
    }
    catch(std::exception &ex)
    {
        Log.e() << "SessionAccountInfoWorker failed: " << Util::formatException(ex);

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->error(toNetworkError(std::current_exception()));
        }
    }
}

void SessionAccountInfoWorker::cancel()
{
    // thread-safe, can be called without locking
    accountClient_.cancel();

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

}
}
