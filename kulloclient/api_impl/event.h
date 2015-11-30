/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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

}
}

std::ostream & operator<<(std::ostream &os, const Kullo::Api::EventType& t);
std::ostream & operator<<(std::ostream &os, const Kullo::Api::Event& e);

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
