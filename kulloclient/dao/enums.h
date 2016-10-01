/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

namespace Kullo {
namespace Dao {

/// Describes the various states that a message can be in.
enum struct MessageState
{
    Read,
    Unread,
    Done,
    Undone
};

/// Describes message filters
enum struct MessagesFilter
{
    Read,
    Unread,
    Done,
    Undone,
    Incoming,
    Outgoing,
    Any
};

/**
 * @brief Describes the various states that a draft can be in.
 */
enum struct DraftState
{
    /// IDraft can be safely edited
    Editing = 0,  // explicit first value, used for iteration

    /// IDraft is enqueued to be sent
    Sending
};

enum struct CreateOld
{
    No = 0,
    Yes
};

enum struct Old
{
    No = 0,
    Yes
};

enum struct IsDraft
{
    No = 0,
    Yes
};

}
}
