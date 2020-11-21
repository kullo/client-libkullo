/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/sync/definitions.h"

#include <iomanip>

#include "kulloclient/util/strings.h"

namespace Kullo {
namespace Sync {

namespace {
std::string percentStr(int64_t part, int64_t whole)
{
    if (whole <= 0) return "?%";

    auto ratio = (float)part / whole;
    std::stringstream result;
    result << std::fixed << std::setprecision(1) << 100 * ratio << "%";
    return result.str();
}
}

bool AttachmentsBlockDownloadProgress::operator==(const AttachmentsBlockDownloadProgress &other) const
{
    return other.downloadedBytes == downloadedBytes
            && other.totalBytes == totalBytes;
}

bool AttachmentsBlockDownloadProgress::operator!=(const AttachmentsBlockDownloadProgress &other) const
{
    return !(*this==other);
}

bool SyncIncomingAttachmentsProgress::operator==(
        const SyncIncomingAttachmentsProgress &other) const
{
    return other.downloadedBytes == downloadedBytes
            && other.totalBytes == totalBytes
            && other.attachmentsBlocks == attachmentsBlocks;
}

bool SyncIncomingAttachmentsProgress::operator!=(
        const SyncIncomingAttachmentsProgress &other) const
{
    return !(*this==other);
}

bool SyncOutgoingMessagesProgress::operator==(
        const SyncOutgoingMessagesProgress &other) const
{
    return other.uploadedBytes == uploadedBytes
            && other.totalBytes == totalBytes;
}

bool SyncOutgoingMessagesProgress::operator!=(
        const SyncOutgoingMessagesProgress &other) const
{
    return !(*this==other);
}

std::ostream &operator<<(std::ostream &out, const SyncIncomingMessagesProgress &rhs)
{
    out << "incoming messages: "
        << rhs.processedMessages << "/" << rhs.totalMessages << " "
        << "(" << percentStr(rhs.processedMessages, rhs.totalMessages) << "), "
        << "new unread: " << rhs.newUnreadMessages << ", "
        << "modified: "   << rhs.modifiedMessages << ", "
        << "deleted: "    << rhs.deletedMessages;
    return out;
}

std::ostream &operator<<(std::ostream &out, const SyncIncomingAttachmentsProgress &rhs)
{
    out << "incoming attachments: "
        << rhs.downloadedBytes << "/" << rhs.totalBytes
        << "B (" << percentStr(rhs.downloadedBytes, rhs.totalBytes) << ") ";

    out << "[";
    bool firstAttachmentElement = true;
    for (const auto &iter: rhs.attachmentsBlocks) {
        auto msgId = iter.first;
        auto downloaded = iter.second.downloadedBytes;
        auto total = iter.second.totalBytes;

        if (!firstAttachmentElement) out << ", ";
        out << msgId << ": " << percentStr(downloaded, total);

        firstAttachmentElement = false;
    }
    out << "]";

    return out;
}

std::ostream &operator<<(std::ostream &out, const SyncOutgoingMessagesProgress &rhs)
{
    out << "outgoing messages: "
        << rhs.uploadedBytes << "/" << rhs.totalBytes
        << "B (" << percentStr(rhs.uploadedBytes, rhs.totalBytes) << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, const SyncProgress &rhs)
{
    out << rhs.incomingMessages << ", "
        << rhs.incomingAttachments << ", "
        << rhs.outgoingMessages << ", "
        << "runtime: " << Util::Strings::formatReadable(rhs.runTimeMs) << "ms";
    return out;
}

}
}
