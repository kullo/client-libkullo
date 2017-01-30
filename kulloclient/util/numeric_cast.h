/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/numeric/conversion/cast.hpp>

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Util {

template<
        typename Target,
        typename Source,
        typename std::enable_if<!std::is_same<Target, Source>::value, int>::type = 0
        > inline
typename boost::numeric::converter<Target,Source>::result_type
numeric_cast(Source arg)
{
    try
    {
        return boost::numeric_cast<Target>(arg);
    }
    catch (boost::bad_numeric_cast &ex)
    {
        kulloAssertionFailed(ex.what());
    }
}

template<typename T> inline
T numeric_cast(T arg)
{
    return arg;
}

}
}
