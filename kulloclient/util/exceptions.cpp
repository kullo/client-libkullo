/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/exceptions.h"

#include <sstream>
#include <typeinfo>

#include <boost/core/demangle.hpp>

namespace Kullo {
namespace Util {

BaseException::BaseException(const std::string &message) throw()
    : message_(message)
{
}

const char *BaseException::what() const throw()
{
    return message_.data();
}

std::string formatException(const std::exception &ex, size_t indent)
{
    std::string result = std::string(indent, ' ')
            + boost::core::demangle(typeid(ex).name());
    auto what = std::string(ex.what());
    if (!what.empty()) result += ": " + what;

    try
    {
        std::rethrow_if_nested(ex);
    }
    catch (std::exception &nested)
    {
        result += "\n" + formatException(nested, indent + 1);
    }
    return result;
}

}
}
