/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/info.h"

#include <typeinfo>

#include <boost/core/demangle.hpp>
#include <botan/botan_all.h>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/strings.h"

namespace Kullo {
namespace Crypto {

namespace {

template<typename T>
const std::type_info &getType(const std::unique_ptr<T> &instance)
{
    kulloAssert(instance);
    auto &instanceRef = *instance;
    return typeid(instanceRef);
}

template<typename GetProviderNames, typename GetInstance>
std::string getImpls(
        GetProviderNames *getProviderNames,
        GetInstance *getInstance,
        const std::string &algorithm)
{
    std::vector<std::string> result;

    const auto &usedImplType = getType(getInstance(algorithm, ""));

    for (const auto &providerName : getProviderNames(algorithm))
    {
        const auto &implType = getType(getInstance(algorithm, providerName));
        auto providerInfo = providerName
                + " (" + boost::core::demangle(implType.name()) + ")";
        if (implType == usedImplType) providerInfo += "*";
        result.push_back(providerInfo);
    }

    return Util::join(result.begin(), result.end(), ", ");
}

}

std::map<std::string, std::string> Info::getImplementationInfos()
{
    std::map<std::string, std::string> out;

    // The providers() interface does not list AES providers anymore
    // https://github.com/randombit/botan/pull/623/files
    out["AES-256"] = Botan::BlockCipher::create("AES-256")->provider();
    out["HMAC(SHA-1)"] = getImpls(
                Botan::MessageAuthenticationCode::providers,
                Botan::MessageAuthenticationCode::create,
                "HMAC(SHA-1)");
    out["HMAC(SHA-512)"] = getImpls(
                Botan::MessageAuthenticationCode::providers,
                Botan::MessageAuthenticationCode::create,
                "HMAC(SHA-512)");
    out["SHA-1"] = getImpls(
                Botan::HashFunction::providers,
                Botan::HashFunction::create,
                "SHA-1");
    out["SHA-512"] = getImpls(
                Botan::HashFunction::providers,
                Botan::HashFunction::create,
                "SHA-512");

    return out;
}

}
}
