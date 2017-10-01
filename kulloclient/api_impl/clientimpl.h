/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/Client.h"
#include "kulloclient/api_impl/logfunctionwrapper.h"

namespace Kullo {
namespace ApiImpl {

class ClientImpl : public Api::Client
{
public:
    ClientImpl();

    nn_shared_ptr<Api::AsyncTask> createSessionAsync(
            const Api::Address &address,
            const Api::MasterKey &masterKey,
            const std::string &dbFilePath,
            const nn_shared_ptr<Api::SessionListener> &sessionListener,
            const nn_shared_ptr<Api::ClientCreateSessionListener> &listener) override;

    nn_shared_ptr<Api::AsyncTask> addressExistsAsync(
            const Api::Address &address,
            const nn_shared_ptr<Api::ClientAddressExistsListener> &listener) override;

    nn_shared_ptr<Api::AsyncTask> checkCredentialsAsync(
            const Api::Address &address,
            const Api::MasterKey &masterKey,
            const nn_shared_ptr<Api::ClientCheckCredentialsListener> &listener) override;

    nn_shared_ptr<Api::AsyncTask> generateKeysAsync(
            const nn_shared_ptr<Api::ClientGenerateKeysListener> &listener) override;

    std::unordered_map<std::string, std::string> versions() override;
};

}
}
