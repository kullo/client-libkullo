/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/clientaddressexistsworker.h"

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/multithreading.h"
#include "kulloclient/registry.h"

namespace Kullo {
namespace ApiImpl {

ClientAddressExistsWorker::ClientAddressExistsWorker(
        std::shared_ptr<Api::Address> address,
        std::shared_ptr<Api::ClientAddressExistsListener> listener)
    : keysClient_(Registry::httpClientFactory()->createHttpClient())
    , address_(address)
    , listener_(listener)
{}

void ClientAddressExistsWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AddrExW");

    // keysClient_ can be used without locking because all
    // non-threadsafe stuff is exclusively used by the worker thread
    try
    {
        keysClient_.getPublicKey(
                    Util::KulloAddress(address_->toString()),
                    Protocol::PublicKeysClient::LATEST_ENCRYPTION_PUBKEY);

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->finished(address_, true);
        }
    }
    catch (Protocol::NotFound)
    {
        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->finished(address_, false);
        }
    }
    catch (std::exception &ex)
    {
        Log.e() << "ClientAddressExistsWorker failed: " << Util::formatException(ex);

        if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
        {
            listener->error(
                        address_,
                        toNetworkError(std::current_exception()));
        }
    }
}

void ClientAddressExistsWorker::cancel()
{
    // thread-safe, can be called without locking
    keysClient_.cancel();

    std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
    listener_.reset();
}

}
}
