/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/clientcheckcredentialsworker.h"

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/masterkey.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

ClientCheckCredentialsWorker::ClientCheckCredentialsWorker(
        std::shared_ptr<Api::Address> address,
        std::shared_ptr<Api::MasterKey> masterKey,
        std::shared_ptr<Api::ClientCheckCredentialsListener> listener)
    : messagesClient_(
          Util::KulloAddress(address->toString()),
          Util::MasterKey(masterKey->dataBlocks()))
    , address_(address)
    , masterKey_(masterKey)
    , listener_(listener)
{}

void ClientCheckCredentialsWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("CheckLoginW");

    try
    {
        // 2^53 = 9007199254740992
        // last date that is exactly converted from and to a 64bit float in JSON
        // 9007199254_unix = 2255-06-05
        messagesClient_.getMessages(lastModified_type{9007199254740992ull});

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_) listener_->finished(address_, masterKey_, true);
    }
    catch(Protocol::Unauthorized)
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_) listener_->finished(address_, masterKey_, false);
    }
    catch(std::exception &ex)
    {
        Log.e() << "ClientCheckLoginWorker failed: " << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        if (listener_)
        {
            listener_->error(
                        address_,
                        toNetworkError(std::current_exception()));
        }
    }
}

void ClientCheckCredentialsWorker::cancel()
{
    // thread-safe, can be called without locking
    messagesClient_.cancel();

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

}
}
