/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/crypto/info.h"

#include <typeinfo>

#include <boost/core/demangle.hpp>
#include <botan/botan.h>

#include "kulloclient/util/assert.h"
#include "kulloclient/util/formatstring.h"

using namespace Botan;

namespace Kullo {
namespace Crypto {

namespace {

template<typename T>
const std::type_info &getTypeAndDelete(T *instance)
{
    auto ptr = std::unique_ptr<T>(instance);
    kulloAssert(ptr);
    return typeid(*instance);
}

template<typename GetProviderNames, typename GetInstance>
std::string getImpls(
        GetProviderNames *getProviderNames,
        GetInstance *getInstance,
        const std::string &algorithm)
{
    std::vector<std::string> result;

    const auto &usedImplType = getTypeAndDelete(getInstance(algorithm, ""));

    for (const auto &providerName : getProviderNames(algorithm))
    {
        const auto &implType = getTypeAndDelete(getInstance(algorithm, providerName));
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
    out["AES-256"] = getImpls(get_block_cipher_providers, get_block_cipher, "AES-256");
    out["HMAC(SHA-1)"] = getImpls(get_mac_providers, get_mac, "HMAC(SHA-1)");
    out["HMAC(SHA-512)"] = getImpls(get_mac_providers, get_mac, "HMAC(SHA-512)");
    out["SHA-1"] = getImpls(get_hash_function_providers, get_hash, "SHA-1");
    out["SHA-512"] = getImpls(get_hash_function_providers, get_hash, "SHA-512");
    return out;
}

}
}
