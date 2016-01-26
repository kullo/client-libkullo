/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <iosfwd>

#include "kulloclient/http/Request.h"
#include "kulloclient/http/Response.h"

namespace Kullo {
namespace Http {

std::string to_string(const Http::HttpMethod &method);
std::ostream &operator<<(std::ostream &os, const Http::HttpMethod &method);
std::ostream &operator<<(std::ostream &os, const Http::Request &req);
std::ostream &operator<<(std::ostream &os, Http::ResponseError err);
std::ostream &operator<<(std::ostream &os, const Http::Response &resp);

}
}
