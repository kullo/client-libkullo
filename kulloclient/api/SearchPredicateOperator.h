// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from messages.djinni

#pragma once

#include <functional>

namespace Kullo { namespace Api {

enum class SearchPredicateOperator : int {
    Is,
    IsNot,
};

} }  // namespace Kullo::Api

namespace std {

template <>
struct hash<::Kullo::Api::SearchPredicateOperator> {
    size_t operator()(::Kullo::Api::SearchPredicateOperator type) const {
        return std::hash<int>()(static_cast<int>(type));
    }
};

}  // namespace std