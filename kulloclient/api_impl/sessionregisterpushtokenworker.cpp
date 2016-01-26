/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/sessionregisterpushtokenworker.h"

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

SessionRegisterPushTokenWorker::SessionRegisterPushTokenWorker(
        const Util::KulloAddress &address,
        const Util::MasterKey &masterKey,
        Operation operation,
        const std::string &registrationToken)
    : operation_(operation)
    , pushClient_(address, masterKey)
    , registrationToken_(registrationToken)
{}

void SessionRegisterPushTokenWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("RegPushTokW");

    try
    {
        switch (operation_)
        {
        case Add:
            pushClient_.addGcmRegistrationToken(registrationToken_);
            break;
        case Remove:
            pushClient_.removeGcmRegistrationToken(registrationToken_);
            break;
        default:
            kulloAssert(false);
        }
    }
    catch(std::exception &ex)
    {
        Log.e() << "SessionRegisterPushTokenWorker failed: "
                << Util::formatException(ex);
    }
}

void SessionRegisterPushTokenWorker::cancel()
{
    // thread-safe, can be called without locking
    pushClient_.cancel();
}

}
}
