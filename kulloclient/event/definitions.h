/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <unordered_set>

#include "kulloclient/api_impl/event.h"

namespace Kullo {
namespace Event {

using ApiEvents = std::unordered_set<Api::Event>;

}
}
