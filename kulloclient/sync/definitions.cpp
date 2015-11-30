/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/sync/definitions.h"

#include <iomanip>

#include "kulloclient/util/formatstring.h"

namespace Kullo {
namespace Sync {

std::ostream &operator<<(std::ostream &out, const SyncMessagesProgress &rhs)
{
    if (rhs.countTotal > 0)
    {
        const auto ratio = (float) rhs.countProcessed / rhs.countTotal;

        std::stringstream percent;
        percent << std::fixed << std::setprecision(1)
                << 100 * ratio << "%";
        out << "[" << percent.str() << "] ";
    }

    out << "processed: "  << rhs.countProcessed << ", "
        << "total: "      << rhs.countTotal << ", "
        << "new unread: " << rhs.countNewUnread << ", "
        << "modified: "   << rhs.countModified << ", "
        << "deleted: "    << rhs.countDeleted;
    return out;
}

std::ostream &operator<<(std::ostream &out, const SyncProgress &rhs)
{
    out << rhs.messages << ", runtime: "
        << Util::FormatString::formatIntegerWithCommas(rhs.runTimeMs).c_str()
        << "ms";
    return out;
}

}
}
