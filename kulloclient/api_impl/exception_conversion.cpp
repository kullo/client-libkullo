/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/exception_conversion.h"

#include <iostream>
#include <boost/filesystem/operations.hpp>

#include "kulloclient/protocol/exceptions.h"
#include "kulloclient/util/assert.h"

namespace fs = boost::filesystem;

namespace Kullo {
namespace ApiImpl {

Api::NetworkError toNetworkError(std::exception_ptr exptr)
{
    kulloAssert(exptr);
    try {std::rethrow_exception(exptr);}
    catch (Protocol::Forbidden)           {return Api::NetworkError::Forbidden;}
    catch (Protocol::ProtocolError)       {return Api::NetworkError::Protocol;}
    catch (Protocol::Unauthorized)        {return Api::NetworkError::Unauthorized;}
    catch (Protocol::InternalServerError) {return Api::NetworkError::Server;}
    catch (...)                       {return Api::NetworkError::Connection;}

    // cannot happen, but make compiler happy
    kulloAssert(false);
    return Api::NetworkError::Connection;
}

Api::LocalError toLocalError(std::exception_ptr exptr)
{
    kulloAssert(exptr);
    try {std::rethrow_exception(exptr);}
    catch (Util::FilesystemError) {return Api::LocalError::Filesystem;}
    catch (fs::filesystem_error)  {return Api::LocalError::Filesystem;}
    catch (std::ios_base::failure){return Api::LocalError::Filesystem;}
    catch (...)                   {return Api::LocalError::Unknown;}

    // cannot happen, but make compiler happy
    kulloAssert(false);
    return Api::LocalError::Unknown;
}

}
}
