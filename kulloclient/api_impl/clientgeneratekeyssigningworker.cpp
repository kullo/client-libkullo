/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/clientgeneratekeyssigningworker.h"

#include "kulloclient/api_impl/clientgeneratekeysworker.h"
#include "kulloclient/crypto/asymmetrickeygenerator.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace ApiImpl {

ClientGenerateKeysSigningWorker::ClientGenerateKeysSigningWorker(
        ClientGenerateKeysWorker &parent)
    : parent_(parent)
{}

void ClientGenerateKeysSigningWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("GenKeysSigningW");

    auto result = make_unique<Crypto::PrivateKey>(
                Crypto::AsymmetricKeyGenerator().makeKeyPair(
                    Crypto::AsymmetricKeyType::Signature));
    parent_.keypairSignature_ = std::move(result);
    parent_.addToProgress(40);
}

void ClientGenerateKeysSigningWorker::cancel()
{
    // can't cancel key generation
}

}
}
