/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/optional.hpp>
#include <cstdint>
#include <vector>

#include "kulloclient/util/datetime.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Protocol {

/**
 * @brief Basic sync result structure
 */
struct IdLastModified
{
    bool operator==(const IdLastModified &rhs) const;
    bool operator<(const IdLastModified &rhs) const;

    /// Server-assigned ID
    id_type id = 0;

    /// Server-assigned modification timestamp
    lastModified_type lastModified = 0;
};

struct Challenge
{
    std::string auth;
    std::string type;
    std::string text;
    std::string user;
    timestamp_type timestamp;
};

/**
 * @brief Message record as returned by the server
 */
struct Message : IdLastModified
{
    /// Returns a string representation for debugging purposes
    std::string toString();

    bool operator==(const Message& rhs) const;

    /// true iff the message was deleted (= is a tombstone)
    bool deleted = false;

    /// The date and time when the server received the message
    /// boost::none if the message was deleted
    boost::optional<Util::DateTime> dateReceived;

    /// The meta data such as read/done that can be modified by the client
    /// (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> meta;

    /// The key safe (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> keySafe;

    /// The message content (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> content;

    /// true iff the message has attachments
    bool hasAttachments = false;
};

struct MessageSent : IdLastModified
{
    /// The date and time when the server received the message
    Util::DateTime dateReceived;

    /// Total size of the request body sent
    std::size_t size = 0;

    MessageSent(Util::DateTime dateReceived)
        : dateReceived(std::move(dateReceived))
    {}
};

/**
 * @brief Attachments record for a message as returned by the server
 */
struct MessageAttachments
{
    /// Server-assigned ID
    id_type id = 0;

    /// Attachment data (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> attachments;
};

/// Message record to be passed to the REST client methods
struct SendableMessage
{
    /// The key safe (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> keySafe;

    /// The message content (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> content;

    /// Attachment data (encrypted, binary, not base64-encoded)
    std::vector<unsigned char> attachments;
};

struct SymmetricKeys
{
    std::string loginKey;
    std::vector<unsigned char> privateDataKey;
};

struct KeyPair final
{
    id_type id = 0;
    std::string type;
    std::vector<unsigned char> pubkey;
    std::vector<unsigned char> privkey;
    Util::DateTime validFrom;
    Util::DateTime validUntil;
    std::vector<unsigned char> revocation;

    KeyPair(Util::DateTime validFrom, Util::DateTime validUntil)
        : validFrom(std::move(validFrom))
        , validUntil(std::move(validUntil))
    {}
};

struct ProfileInfo
{
    std::string key;
    std::string value;
    lastModified_type lastModified;
};

}
}
