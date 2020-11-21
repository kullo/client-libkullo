/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include "kulloclient/api_impl/worker.h"

namespace Kullo {
namespace ApiImpl {

class ClientGenerateKeysWorker;

class ClientGenerateKeysSigningWorker : public Worker
{
public:
    ClientGenerateKeysSigningWorker(ClientGenerateKeysWorker &parent);

    void work() override;
    void cancel() override;

private:
    ClientGenerateKeysWorker &parent_;
};

}
}
