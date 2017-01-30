/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <iosfwd>

namespace Kullo {
namespace Sync {

enum struct SyncMode { SendOnly, WithoutAttachments, Everything };
enum struct SyncPhase {
    Keys,
    Profile,
    IncomingMessages,
    IncomingAttachments,
    OutgoingMessages,
};

struct SyncIncomingMessagesProgress
{
    int64_t processedMessages = 0;
    int64_t totalMessages = 0;
    int64_t newMessages = 0;
    int64_t newUnreadMessages = 0;
    int64_t modifiedMessages = 0;
    int64_t deletedMessages = 0;
};

struct SyncIncomingAttachmentsProgress
{
    int64_t downloadedBytes = 0;
    int64_t totalBytes = 0;

    bool operator==(const SyncIncomingAttachmentsProgress &other) const;
    bool operator!=(const SyncIncomingAttachmentsProgress &other) const;
};

struct SyncOutgoingMessagesProgress
{
    int64_t uploadedBytes = 0;
    int64_t totalBytes = 0;

    bool operator==(const SyncOutgoingMessagesProgress &other) const;
    bool operator!=(const SyncOutgoingMessagesProgress &other) const;
};

struct SyncProgress
{
    SyncPhase phase;
    SyncIncomingMessagesProgress incomingMessages;
    SyncIncomingAttachmentsProgress incomingAttachments;
    SyncOutgoingMessagesProgress outgoingMessages;
    int64_t runTimeMs = 0;
};

std::ostream &operator<<(std::ostream &out, const SyncIncomingMessagesProgress &rhs);
std::ostream &operator<<(std::ostream &out, const SyncIncomingAttachmentsProgress &rhs);
std::ostream &operator<<(std::ostream &out, const SyncOutgoingMessagesProgress &rhs);
std::ostream &operator<<(std::ostream &out, const SyncProgress &rhs);

}
}
