/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/delivery.h"

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Util {

Delivery::Delivery(const KulloAddress &recipient, Delivery::State state) :
    recipient(recipient),
    state(state)
{
}

bool Delivery::operator<(const Delivery &other) const
{
    return recipient < other.recipient;
}

bool Delivery::operator==(const Delivery &other) const
{
    return
            recipient == other.recipient &&
            state == other.state &&
            reason == other.reason &&
            date == other.date &&
            lockHolder == other.lockHolder &&
            lockExpires == other.lockExpires;
}

Delivery::State Delivery::toState(const std::string &str)
{
    if (str == "unsent") return State::unsent;
    if (str == "delivered") return State::delivered;
    if (str == "failed") return State::failed;
    throw std::invalid_argument(
                std::string("[Delivery::stringToState] Invalid state: ") + str);
}

std::string Delivery::toString(Delivery::State state)
{
    switch (state)
    {
    case unsent: return "unsent";
    case delivered: return "delivered";
    case failed: return "failed";
    default:
        kulloAssert(false);
        return ""; // make compiler happy
    }
}

Delivery::Reason Delivery::toReason(const std::string &str)
{
    if (str == "unknown") return Reason::unknown;
    if (str == "doesnt_exist") return Reason::doesnt_exist;
    if (str == "too_large") return Reason::too_large;
    if (str == "canceled") return Reason::canceled;
    throw std::invalid_argument(
                std::string("[Delivery::stringToReason] Invalid reason: ") + str);
}

std::string Delivery::toString(Delivery::Reason reason)
{
    switch (reason)
    {
    case unknown: return "unknown";
    case doesnt_exist: return "doesnt_exist";
    case too_large: return "too_large";
    case canceled: return "canceled";
    default:
        kulloAssert(false);
        return ""; // make compiler happy
    }
}

}
}
