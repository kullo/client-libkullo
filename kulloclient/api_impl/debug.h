/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <iostream>

#include "kulloclient/api/DeliveryReason.h"
#include "kulloclient/api/DeliveryState.h"
#include "kulloclient/api/DraftPart.h"
#include "kulloclient/api/LocalError.h"
#include "kulloclient/api/MessagesSearchResult.h"
#include "kulloclient/api/NetworkError.h"
#include "kulloclient/api/SearchPredicateOperator.h"
#include "kulloclient/api/SenderPredicate.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Api {

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

inline std::ostream &operator<<(std::ostream &os, Api::DraftPart value)
{
    switch (value)
    {
    case Api::DraftPart::Attachments:
        os << "Attachments";
        break;
    case Api::DraftPart::Content:
        os << "Content";
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
    case Api::LocalError::FileTooBig:
        os << "FileTooBig";
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

inline std::ostream &operator<<(std::ostream &os, Api::MessagesSearchResult value)
{
    os << "MessagesSearchResult("
       << "msgId=" << value.msgId << ", "
       << "convId=" << value.convId << ", "
       << "senderAddress=" << value.senderAddress << ", "
       << "dateReceived=" << value.dateReceived << ", "
       << "snippet=\"" << value.snippet << "\")";
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Api::SearchPredicateOperator value)
{
    switch (value)
    {
    case Api::SearchPredicateOperator::Is:
        os << "Is";
        break;
    case Api::SearchPredicateOperator::IsNot:
        os << "IsNot";
        break;
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, Api::SenderPredicate value)
{
    os << "SenderPredicate("
       << "predicateOperator=" << value.predicateOperator << ", "
       << "senderAddress=\"" << value.senderAddress << "\")";
    return os;
}

}
}
