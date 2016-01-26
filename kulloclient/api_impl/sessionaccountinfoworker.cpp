/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sessionaccountinfoworker.h"

#include "kulloclient/api/AccountInfo.h"
#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

SessionAccountInfoWorker::SessionAccountInfoWorker(
        const Util::KulloAddress &address,
        const Util::MasterKey &masterKey,
        std::shared_ptr<Api::SessionAccountInfoListener> listener)
    : accountClient_(address, masterKey)
    , listener_(listener)
{}

void SessionAccountInfoWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AccountInfoW");

    try
    {
        auto settingsLocation = accountClient_.getSettingsLocation();

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_) listener_->finished(Api::AccountInfo(settingsLocation));
    }
    catch(std::exception &ex)
    {
        Log.e() << "SessionAccountInfoWorker failed: " << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(toNetworkError(std::current_exception()));
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
