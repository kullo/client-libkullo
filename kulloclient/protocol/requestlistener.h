/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
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
