/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <string>
#include <vector>
#include <jsoncpp/jsoncpp-forwards.h>

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Util {

class JsonException : public BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    JsonException(const std::string &message) throw() : BaseException(message) {}
    ~JsonException() = default;
};

class JsonHelper
{
public:
    static Json::Value readObject(const std::string &json);

    static Json::Value stringsToJsonArray(const std::vector<std::string> &in);
};

}
}
