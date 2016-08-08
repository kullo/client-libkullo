/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <iostream>

#include "kulloclient/api/Address.h"
#include "kulloclient/api/DeliveryReason.h"
#include "kulloclient/api/DeliveryState.h"
#include "kulloclient/api/LocalError.h"
#include "kulloclient/api/NetworkError.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Api {

// cannot use const& because toString is Djinni-defined and therefore not const
inline std::ostream &operator<<(std::ostream &os, Api::Address &value)
{
    os << value.toString();
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Api::DeliveryReason value)
{
    switch (value)
    {
    case Api::DeliveryReason::Unknown:
        os << "Unknown";
        break;
    case Api::DeliveryReason::DoesntExist:
        os << "DoesntExist";
        break;
    case Api::DeliveryReason::TooLarge:
        os << "TooLarge";
        break;
    case Api::DeliveryReason::Canceled:
        os << "Canceled";
        break;
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Api::DeliveryState value)
{
    switch (value)
    {
    case Api::DeliveryState::Unsent:
        os << "Unsent";
        break;
    case Api::DeliveryState::Delivered:
        os << "Delivered";
        break;
    case Api::DeliveryState::Failed:
        os << "Failed";
        break;
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Api::LocalError value)
{
    switch (value)
    {
    case Api::LocalError::Filesystem:
        os << "Filesystem";
        break;
    case Api::LocalError::Unknown:
        os << "Unknown";
        break;
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Api::NetworkError value)
{
    switch (value)
    {
    case Api::NetworkError::Connection:
        os << "Connection";
        break;
    case Api::NetworkError::Forbidden:
        os << "Forbidden";
        break;
    case Api::NetworkError::Protocol:
        os << "Protocol";
        break;
    case Api::NetworkError::Server:
        os << "Server";
        break;
    case Api::NetworkError::Unauthorized:
        os << "Unauthorized";
        break;
    case Api::NetworkError::Unknown:
        os << "Unknown";
        break;
    }
    return os;
}

}
}
