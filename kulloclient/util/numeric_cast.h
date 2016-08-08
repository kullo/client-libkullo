/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/numeric/conversion/cast.hpp>

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Util {

template<typename Target, typename Source> inline
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

}
}
