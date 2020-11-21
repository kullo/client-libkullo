/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/registry.h"

#include "kulloclient/util/misc.h"
#include "kulloclient/http/Registry.h"

namespace Kullo {

std::shared_ptr<ApiImpl::LogFunctionWrapper> Registry::logFunctionWrapper_;
std::shared_ptr<Api::TaskRunner> Registry::taskRunner_;

void Registry::setLogListener(const std::shared_ptr<Api::LogListener> &listener)
{
    if (!listener)
    {
        logFunctionWrapper_.reset();
    }
    else
    {
        logFunctionWrapper_ = make_unique<ApiImpl::LogFunctionWrapper>(listener);
    }
}

void Registry::setTaskRunner(const std::shared_ptr<Api::TaskRunner> &taskRunner)
{
    taskRunner_ = taskRunner;
}

std::shared_ptr<Api::TaskRunner> Registry::taskRunner()
{
    kulloAssert(taskRunner_);
    return taskRunner_;
}


// implementation of static methods of Http::Registry
namespace Http {
void Registry::setHttpClientFactory(
        const nn_shared_ptr<HttpClientFactory> &factory)
{
    ::Kullo::Registry::setHttpClientFactory(factory);
}
} // namespace Http


std::shared_ptr<Http::HttpClientFactory> Registry::httpClientFactory_;

void Registry::setHttpClientFactory(
        const std::shared_ptr<Kullo::Http::HttpClientFactory> &factory)
{
    httpClientFactory_ = factory;
}

std::shared_ptr<Http::HttpClientFactory> Registry::httpClientFactory()
{
    return httpClientFactory_;
}

}
