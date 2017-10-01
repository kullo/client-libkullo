// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from address.djinni

#pragma once

#include <string>
#include <utility>

namespace Kullo { namespace Api {

struct Address; // Requiring extended class

/** Represents a Kullo address */
struct AddressBase {
    /** Constructor must ensure this is normalized */
    std::string localPart;
    /** Constructor must ensure this is normalized */
    std::string domainPart;

    AddressBase(std::string localPart_,
                std::string domainPart_)
    : localPart(std::move(localPart_))
    , domainPart(std::move(domainPart_))
    {}

    virtual ~AddressBase() = default;

protected:
    AddressBase(const AddressBase&) = default;
#if !defined(_MSC_VER) || _MSC_VER > 1900
    AddressBase(AddressBase&&) = default;
#endif
    AddressBase& operator=(const AddressBase&) = default;
#if !defined(_MSC_VER) || _MSC_VER > 1900
    AddressBase& operator=(AddressBase&&) = default;
#endif
};

} }  // namespace Kullo::Api
