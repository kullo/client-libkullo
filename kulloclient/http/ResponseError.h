// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#pragma once

#include <functional>

namespace Kullo { namespace Http {

enum class ResponseError : int {
    Canceled,
    Timeout,
    NetworkError,
};

} }  // namespace Kullo::Http

namespace std {

template <>
struct hash<::Kullo::Http::ResponseError> {
    size_t operator()(::Kullo::Http::ResponseError type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std
