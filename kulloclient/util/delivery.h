/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional.hpp>
#include <string>

#include "kulloclient/util/datetime.h"
#include "kulloclient/util/kulloaddress.h"

namespace Kullo {
namespace Util {

struct Delivery
{
    enum State
    {
        unsent = 0,
        delivered,
        failed
    };

    enum Reason
    {
        unknown = 0,
        doesnt_exist,
        too_large,
        canceled
    };

    KulloAddress recipient;
    State state;
    boost::optional<Reason> reason;
    boost::optional<DateTime> date;
    boost::optional<std::string> lockHolder;
    boost::optional<DateTime> lockExpires;

    Delivery(const Util::KulloAddress &recipient, State state);
    bool operator<(const Delivery &other) const;
    bool operator==(const Delivery &other) const;

    static State toState(const std::string &str);
    static std::string toString(State state);
    static Reason toReason(const std::string &str);
    static std::string toString(Reason reason);
};

}
}
