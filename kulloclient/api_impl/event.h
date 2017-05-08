/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>

#include "kulloclient/api/Event.h"

namespace Kullo {
namespace Api {

inline bool operator==(const Event &lhs, const Event &rhs)
{
    return
            lhs.event == rhs.event &&
            lhs.conversationId == rhs.conversationId &&
            lhs.messageId == rhs.messageId &&
            lhs.attachmentId == rhs.attachmentId;
}

inline bool operator<(const Event &lhs, const Event &rhs)
{
    return
            lhs.event < rhs.event &&
            lhs.conversationId < rhs.conversationId &&
            lhs.messageId < rhs.messageId &&
            lhs.attachmentId < rhs.attachmentId;
}

// "It's important that the << operator is defined in the SAME
// namespace that defines Bar.  C++'s look-up rules rely on that."
// https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#teaching-google-test-how-to-print-your-values
std::ostream & operator<<(std::ostream &os, const Kullo::Api::EventType& t);
std::ostream & operator<<(std::ostream &os, const Kullo::Api::Event& e);

}
}


namespace std {
template <>
struct hash<Kullo::Api::Event> {
    size_t operator()(Kullo::Api::Event type) const {
        return std::hash<decltype(type.event)>()(type.event) ^
               std::hash<decltype(type.conversationId)>()(type.conversationId) ^
               std::hash<decltype(type.messageId)>()(type.messageId) ^
               std::hash<decltype(type.attachmentId)>()(type.attachmentId);
    }
};
}
