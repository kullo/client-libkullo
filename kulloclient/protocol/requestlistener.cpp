/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/protocol/requestlistener.h"

namespace Kullo {
namespace Protocol {

RequestListener::RequestListener()
    : Http::RequestListener(),
      read_([](int64_t){return std::vector<uint8_t>();})
{
}

std::vector<uint8_t> RequestListener::read(int64_t maxSize)
{
    return read_(maxSize);
}

void RequestListener::setRead(const RequestListener::Read &func)
{
    read_ = func;
}

}
}
