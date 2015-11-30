/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <iostream>
#include <boost/optional/optional_io.hpp>

#include "kulloclient/http/Request.h"
#include "kulloclient/http/Response.h"
#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Http {

std::string to_string(const Http::HttpMethod &method)
{
    switch (method)
    {
    case Http::HttpMethod::Get:
        return "GET";
    case Http::HttpMethod::Post:
        return "POST";
    case Http::HttpMethod::Put:
        return "PUT";
    case Http::HttpMethod::Patch:
        return "PATCH";
    case Http::HttpMethod::Delete:
        return "DELETE";
    default:
        kulloAssert(false);
        return "";
    }}

std::ostream &operator<<(std::ostream &os, const Http::HttpMethod &method)
{
    os << to_string(method);
    return os;
}

std::ostream &operator<<(std::ostream &os, const Http::Request &req)
{
    os << "Request{Method: " << req.method
       << ", URL: " << req.url << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, Http::ResponseError err)
{
    switch (err)
    {
    case Http::ResponseError::Canceled:
        os << "Canceled";
        break;
    case Http::ResponseError::NetworkError:
        os << "NetworkError";
        break;
    case Http::ResponseError::Timeout:
        os << "Timeout";
        break;
    default:
        os << "<unknown>";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Http::Response &resp)
{
    auto &r = const_cast<Http::Response&>(resp);
    os << "Response{Status: " << r.statusCode
       << ", Error: " << r.error << "}";
    return os;
}

}
}
