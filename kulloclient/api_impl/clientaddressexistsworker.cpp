/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/clientaddressexistsworker.h"

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/util/kulloaddress.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/protocol/exceptions.h"

namespace Kullo {
namespace ApiImpl {

ClientAddressExistsWorker::ClientAddressExistsWorker(
        std::shared_ptr<Api::Address> address,
        std::shared_ptr<Api::ClientAddressExistsListener> listener)
    : address_(address)
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

        std::lock_guard<std::mutex> lock(mutex_); (void)lock;
        if (listener_) listener_->finished(address_, true);
    }
    catch (Protocol::NotFound)
    {
        std::lock_guard<std::mutex> lock(mutex_); (void)lock;
        if (listener_) listener_->finished(address_, false);
    }
    catch (std::exception &ex)
    {
        Log.e() << "ClientAddressExistsWorker failed: " << Util::formatException(ex);

        std::lock_guard<std::mutex> lock(mutex_); (void)lock;
        if (listener_)
        {
            listener_->error(
                        address_,
                        toNetworkError(std::current_exception()));
        }
    }
}

void ClientAddressExistsWorker::cancel()
{
    // thread-safe, can be called without locking
    keysClient_.cancel();

    std::lock_guard<std::mutex> lock(mutex_); (void)lock;
    listener_.reset();
}

}
}
