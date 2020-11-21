/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/util/jsonhelper.h"

#include <jsoncpp/jsoncpp.h>

namespace Kullo {
namespace Util {

Json::Value JsonHelper::readObject(const std::string &json)
{
    Json::Reader jsonReader(Json::Features::strictMode());
    Json::Value root;
    if (!jsonReader.parse(json, root, false))
    {
        throw JsonException(jsonReader.getFormattedErrorMessages());
    }
    if (!root.isObject())
    {
        throw JsonException("root is not an object");
    }
    return root;
}

Json::Value JsonHelper::stringsToJsonArray(const std::vector<std::string> &in)
{
    Json::Value out(Json::arrayValue);
    for (const auto &str: in)
    {
        out.append(str);
    }
    return out;
}

}
}
