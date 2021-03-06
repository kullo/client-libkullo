/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <memory>

#include "kulloclient/api/TaskRunner.h"
#include "kulloclient/api_impl/logfunctionwrapper.h"
#include "kulloclient/http/HttpClientFactory.h"

namespace Kullo {

class Registry
{
public:
    // Api::Registry
    static void setLogListener(const std::shared_ptr<Api::LogListener> &listener);
    static void setTaskRunner(const std::shared_ptr<Api::TaskRunner> &taskRunner);

    static std::shared_ptr<Api::TaskRunner> taskRunner();

    // Http::Registry
    static void setHttpClientFactory(
            const std::shared_ptr<Http::HttpClientFactory> &factory);

    static std::shared_ptr<Http::HttpClientFactory> httpClientFactory();

private:
    static std::shared_ptr<ApiImpl::LogFunctionWrapper> logFunctionWrapper_;
    static std::shared_ptr<Api::TaskRunner> taskRunner_;
    static std::shared_ptr<Http::HttpClientFactory> httpClientFactory_;
};

}
