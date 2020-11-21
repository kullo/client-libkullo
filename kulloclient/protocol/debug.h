/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
