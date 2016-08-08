/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sessionregisterpushtokenworker.h"

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/registry.h"

namespace Kullo {
namespace ApiImpl {

namespace {

const auto WAIT_DURATION = std::chrono::seconds(10);

}

SessionRegisterPushTokenWorker::SessionRegisterPushTokenWorker(
        const Util::KulloAddress &address,
        const Util::MasterKey &masterKey,
        Operation operation,
        const Api::PushToken &token)
    : operation_(operation)
    , pushClient_(
          address,
          masterKey,
          Registry::httpClientFactory()->createHttpClient())
    , token_(token)
{}

void SessionRegisterPushTokenWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("RegPushTokW");

    while (true)
    {
        try
        {
            switch (operation_)
            {
            case Add:
                pushClient_.addGcmRegistrationToken(token_);
                break;
            case Remove:
                pushClient_.removeGcmRegistrationToken(token_);
                break;
            }
            break;
        }
        catch (Protocol::NetworkError &ex)
        {
            Log.w() << "NetworkError: " << Util::formatException(ex);
        }
        catch (Protocol::Timeout &ex)
        {
            Log.w() << "Timeout: " << Util::formatException(ex);
        }
        catch (Protocol::ServerError &ex)
        {
            Log.w() << "Server error: " << Util::formatException(ex);
        }
        catch (std::exception &ex)
        {
            Log.e() << "SessionRegisterPushTokenWorker failed: "
                    << Util::formatException(ex);
            break;
        }

        // One of the protocol exceptions listed above has been thrown.
        // Retry the request after getting a bit of sleep, if it has not been
        // cancelled in the meantime.
        std::unique_lock<std::mutex> lock(cancelMutex_);
        if (cancelCv_.wait_for(lock, WAIT_DURATION, [this]{ return cancelled_; }))
        {
            // operation has been cancelled
            break;
        }
    }
}

void SessionRegisterPushTokenWorker::cancel()
{
    // thread-safe, can be called without locking
    pushClient_.cancel();

    // notify sleeping worker
    {
        std::lock_guard<std::mutex> lock(cancelMutex_); K_RAII(lock);
        cancelled_ = true;
    }
    cancelCv_.notify_one();
}

}
}
