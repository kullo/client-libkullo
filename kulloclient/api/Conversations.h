// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from conversations.djinni

#pragma once

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

namespace Kullo { namespace Api {

class Address;
struct DateTime;

class Conversations {
public:
    virtual ~Conversations() {}

    /** Returns all conversation IDs in no particular order */
    virtual std::vector<int64_t> all() = 0;

    /**
     * Returns the conversation with the given participants (excluding the local
     * user), or -1 if the conversation doesn't exist
     */
    virtual int64_t get(const std::unordered_set<std::shared_ptr<Address>> & participants) = 0;

    /**
     * Adds a new conversation with the given participants (excluding the local
     * user) if it doesn't exist yet. Returns its ID in either case. All
     * addresses should have been validated through Client::addressExistsAsync()
     * before passing them into add().
     */
    virtual int64_t add(const std::unordered_set<std::shared_ptr<Address>> & participants) = 0;

    /**
     * Triggers removal of the given conversation. This will also remove all
     * dependencies (messages, drafts, ...). Removal happens asynchronously after
     * calling this method.
     */
    virtual void triggerRemoval(int64_t convId) = 0;

    /** Returns the participants (excluding the local user) */
    virtual std::unordered_set<std::shared_ptr<Address>> participants(int64_t convId) = 0;

    /** Total number of messages */
    virtual int32_t totalMessages(int64_t convId) = 0;

    /** Total number of unread messages */
    virtual int32_t unreadMessages(int64_t convId) = 0;

    /** Total number of undone messages */
    virtual int32_t undoneMessages(int64_t convId) = 0;

    /** Total number of incoming messages */
    virtual int32_t incomingMessages(int64_t convId) = 0;

    /** Total number of outgoing messages */
    virtual int32_t outgoingMessages(int64_t convId) = 0;

    /**
     * Timestamp of the latest message (for sorting). Returns the result of
     * emptyConversationTimestamp() if the conversation has no messages.
     */
    virtual DateTime latestMessageTimestamp(int64_t convId) = 0;

    /** A date many years in the future, used in latestMessageTimestamp */
    static DateTime emptyConversationTimestamp();
};

} }  // namespace Kullo::Api
