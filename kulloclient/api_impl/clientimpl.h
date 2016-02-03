/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/api/Client.h"
#include "kulloclient/api_impl/logfunctionwrapper.h"

namespace Kullo {
namespace ApiImpl {

class ClientImpl : public Api::Client
{
public:
    ClientImpl();

    std::shared_ptr<Api::AsyncTask> createSessionAsync(
            const std::shared_ptr<Api::UserSettings> &settings,
            const std::string &dbFilePath,
            const std::shared_ptr<Api::SessionListener> &sessionListener,
            const std::shared_ptr<Api::ClientCreateSessionListener> &listener) override;

    std::shared_ptr<Api::AsyncTask> addressExistsAsync(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::ClientAddressExistsListener> &listener) override;

    std::shared_ptr<Api::AsyncTask> checkCredentialsAsync(
            const std::shared_ptr<Api::Address> &address,
            const std::shared_ptr<Api::MasterKey> &masterKey,
            const std::shared_ptr<Api::ClientCheckCredentialsListener> &listener) override;

    std::shared_ptr<Api::AsyncTask> generateKeysAsync(
            const std::shared_ptr<Api::ClientGenerateKeysListener> &listener) override;

    std::unordered_map<std::string, std::string> versions() override;
};

}
}
