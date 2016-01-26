/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <iosfwd>

namespace Kullo {
namespace Sync {

enum struct SyncMode { SendOnly, WithoutAttachments, Everything };

struct SyncMessagesProgress
{
    int64_t countLeft = -1;
    int64_t countProcessed = 0;
    int64_t countTotal = -1;
    int64_t countNew = 0;
    int64_t countNewUnread = 0;
    int64_t countModified = 0;
    int64_t countDeleted = 0;
};

struct SyncProgress
{
    SyncMessagesProgress messages;
    int64_t runTimeMs = 0;
};

std::ostream &operator<<(std::ostream &out, const SyncMessagesProgress &rhs);
std::ostream &operator<<(std::ostream &out, const SyncProgress &rhs);

}
}
