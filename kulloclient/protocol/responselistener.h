/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <functional>

#include "kulloclient/http/ResponseListener.h"

namespace Kullo {
namespace Protocol {

class ResponseListener : public Http::ResponseListener
{
public:
    ResponseListener();

    using DataReceived = std::function<void(const std::vector<uint8_t> &)>;
    using ShouldCancel = std::function<bool()>;

    Http::ProgressResult progress(
            int64_t uploadTransferred, int64_t uploadTotal,
            int64_t downloadTransferred, int64_t downloadTotal) override;
    void dataReceived(const std::vector<uint8_t> &data) override;

    void setDataReceived(const DataReceived &func);
    void setShouldCancel(const ShouldCancel &func);

private:
    DataReceived dataReceived_;
    ShouldCancel shouldCancel_;
};

}
}
