/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/clientgeneratekeysworker.h"

#include <atomic>
#include <future>
#include <botan/botan_all.h>

#include "kulloclient/api_impl/asynctaskimpl.h"
#include "kulloclient/api_impl/clientgeneratekeyssigningworker.h"
#include "kulloclient/api_impl/registrationimpl.h"
#include "kulloclient/crypto/asymmetrickeygenerator.h"
#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/multithreading.h"
#include "kulloclient/util/scopedbenchmark.h"

namespace Kullo {
namespace ApiImpl {

ClientGenerateKeysWorker::ClientGenerateKeysWorker(
        std::shared_ptr<Api::ClientGenerateKeysListener> listener)
    : listener_(listener)
    , progress_(0)
{}

void ClientGenerateKeysWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("GenKeysW");

    std::shared_ptr<RegistrationImpl> registration;

    {
        Util::ScopedBenchmark benchmark("Key generation");
        K_RAII(benchmark);

        // trigger initial progress report
        addToProgress(0);
        if (cancelGenKeysRequested_) return;

        // generate masterKey
        auto masterKey = make_unique<Crypto::SymmetricKey>(
                    Crypto::SymmetricKeyGenerator().makeMasterKey());
        addToProgress(10);
        if (cancelGenKeysRequested_) return;

        // Start generation of keypairSignature in another thread.
        // NOTE: Due to concurrency issues in Botan::EntropySource::poll_available_sources,
        // this must only run after the first key has been generated.
        keypairSignature_.reset();
        AsyncTaskImpl keypairSignatureTask(
                    std::make_shared<ClientGenerateKeysSigningWorker>(*this));

        // generate privateDataKey
        auto privateDataKey = make_unique<Crypto::SymmetricKey>(
                    Crypto::SymmetricKeyGenerator().makePrivateDataKey());
        addToProgress(10);
        if (cancelGenKeysRequested_) return;

        // generate keypairEncryption
        auto keypairEncryption = make_unique<Crypto::PrivateKey>(
                    Crypto::AsymmetricKeyGenerator().makeKeyPair(
                        Crypto::AsymmetricKeyType::Encryption));
        addToProgress(40);
        if (cancelGenKeysRequested_) return;

        // wait for keypairSignature
        keypairSignatureTask.waitUntilDone();
        kulloAssert(keypairSignature_);

        registration = std::make_shared<RegistrationImpl>(
                    std::move(masterKey),
                    std::move(privateDataKey),
                    std::move(keypairEncryption),
                    std::move(keypairSignature_));
    }

    // notify listener of completion
    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
    {
        listener->finished(NN_CHECK_ASSERT(registration));
    }
}

void ClientGenerateKeysWorker::cancel()
{
    cancelGenKeysRequested_ = true;
}

void ClientGenerateKeysWorker::addToProgress(
        int8_t progressGrowth)
{
    progress_ += progressGrowth;

    // notify listener of completion
    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_))
    {
        listener->progress(progress_);
    }
}

}
}
