/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
