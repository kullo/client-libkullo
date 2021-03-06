// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from registration.djinni

#pragma once

#include <functional>

namespace Kullo { namespace Api {

enum class ChallengeType : int {
    Code,
    Reservation,
    Reset,
};

} }  // namespace Kullo::Api

namespace std {

template <>
struct hash<::Kullo::Api::ChallengeType> {
    size_t operator()(::Kullo::Api::ChallengeType type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std
