/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <stdint.h>

namespace Kullo {
namespace Data {

struct DraftAttachment
{
    const uint64_t convId;
    const uint64_t attId;

    DraftAttachment(uint64_t convId, uint64_t attId)
        : convId(convId), attId(attId)
    {}
};

}
}
