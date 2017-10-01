/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/Client.h"

#include "kulloclient/api_impl/clientimpl.h"

namespace Kullo {
namespace Api {

nn_shared_ptr<Client> Client::create()
{
    return nn_make_shared<ApiImpl::ClientImpl>();
}

}
}
