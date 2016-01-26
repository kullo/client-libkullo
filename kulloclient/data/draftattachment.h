/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
