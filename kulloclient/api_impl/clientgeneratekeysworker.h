/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <atomic>
#include <memory>

#include "kulloclient/api/ClientGenerateKeysListener.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/crypto/privatekey.h"

namespace Kullo {
namespace ApiImpl {

class ClientGenerateKeysWorker : public Worker
{
public:
    ClientGenerateKeysWorker(
            std::shared_ptr<Api::ClientGenerateKeysListener> listener);

    void work() override;
    void cancel() override;

private:
    void addToProgress(int8_t progressGrowth);

    friend class ClientGenerateKeysSigningWorker;

    // all uses are inherently sequential, thus must not be synchronized
    std::unique_ptr<Crypto::PrivateKey> keypairSignature_;

    // all uses must be synchronized
    std::shared_ptr<Api::ClientGenerateKeysListener> listener_;

    // only atomic operations
    std::atomic<int8_t> progress_;
    std::atomic<bool> cancelGenKeysRequested_{false};
};

}
}
