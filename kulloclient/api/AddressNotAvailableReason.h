// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from registration.djinni

#pragma once

#include <functional>

namespace Kullo { namespace Api {

enum class AddressNotAvailableReason : int {
    Blocked,
    Exists,
};

} }  // namespace Kullo::Api

namespace std {

template <>
struct hash<::Kullo::Api::AddressNotAvailableReason> {
    size_t operator()(::Kullo::Api::AddressNotAvailableReason type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std
