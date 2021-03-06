/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
    /// draft can be safely edited
    Editing = 0,  // value must not be changed because DB relies on it

    /// draft is enqueued to be sent
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
