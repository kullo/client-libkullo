/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <functional>

#include "kulloclient/http/RequestListener.h"

namespace Kullo {
namespace Protocol {

class RequestListener : public Http::RequestListener
{
public:
    RequestListener();

    using Read = std::function<std::vector<uint8_t>(int64_t)>;

    virtual std::vector<uint8_t> read(int64_t maxSize) override;

    void setRead(const Read &func);

private:
    Read read_;
};

}
}
