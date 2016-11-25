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

    using Progressed = std::function<
        Http::ProgressResult(const Http::TransferProgress &progress)
    >;
    using DataReceived = std::function<void(const std::vector<uint8_t> &)>;

    Http::ProgressResult progressed(const Http::TransferProgress &progress) override;
    void dataReceived(const std::vector<uint8_t> &data) override;

    void setProgressed(const Progressed &func);
    void setDataReceived(const DataReceived &func);

private:
    Progressed progressed_;
    DataReceived dataReceived_;
};

}
}
