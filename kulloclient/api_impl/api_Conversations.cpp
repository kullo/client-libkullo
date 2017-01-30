/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api/Conversations.h"

#include "kulloclient/api_impl/DateTime.h"

namespace Kullo {
namespace Api {

DateTime Conversations::emptyConversationTimestamp()
{
    return {2222, 2, 22, 22, 22, 22, 0};
}

}
}
