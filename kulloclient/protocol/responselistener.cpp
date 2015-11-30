/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/protocol/responselistener.h"

#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Protocol {

ResponseListener::ResponseListener()
    : Http::ResponseListener(),
      dataReceived_([](const std::vector<uint8_t> &){}),
      shouldCancel_([](){return false;})
{
}

Http::ProgressResult ResponseListener::progress(
        int64_t uploadTransferred, int64_t uploadTotal,
        int64_t downloadTransferred, int64_t downloadTotal)
{
    K_UNUSED(uploadTransferred);
    K_UNUSED(uploadTotal);
    K_UNUSED(downloadTransferred);
    K_UNUSED(downloadTotal);

    if (shouldCancel_())
    {
        return Http::ProgressResult::Cancel;
    }
    return Http::ProgressResult::Continue;
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

void ResponseListener::setShouldCancel(
        const ResponseListener::ShouldCancel &func)
{
    shouldCancel_ = func;
}

}
}
