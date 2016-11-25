/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/responselistener.h"

#include "kulloclient/http/ProgressResult.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Protocol {

ResponseListener::ResponseListener()
    : Http::ResponseListener(),
      progressed_([](const Http::TransferProgress &)
        {
            return Http::ProgressResult::Continue;
        }),
      dataReceived_([](const std::vector<uint8_t> &){})
{
}

Http::ProgressResult ResponseListener::progressed(
        const Http::TransferProgress &progress)
{
    return progressed_(progress);
}

void ResponseListener::dataReceived(const std::vector<uint8_t> &data)
{
    dataReceived_(data);
}

void ResponseListener::setDataReceived(
        const ResponseListener::DataReceived &func)
{
    dataReceived_ = func;
}

void ResponseListener::setProgressed(
        const ResponseListener::Progressed &func)
{
    progressed_ = func;
}

}
}
