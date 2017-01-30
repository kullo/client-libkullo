/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Protocol {

/// Base class for all HTTP exceptions
class HttpError : public Util::BaseException
{
protected:
    /// @copydoc BaseException::BaseException(const std::string&)
    HttpError(const std::string &message = "") throw() : BaseException(message) {}
};

class UnhandledHttpStatus : public HttpError
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    UnhandledHttpStatus(int32_t statusCode) throw()
        : HttpError(std::string("HTTP ") + std::to_string(statusCode))
    {}
};

// HTTP 401 Unauthorized
class Unauthorized : public HttpError
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    Unauthorized(const std::string &message = "") throw() : HttpError(message) {}
};

// HTTP 403 Forbidden
class Forbidden : public HttpError
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    Forbidden(const std::string &message = "") throw() : HttpError(message) {}
};

// HTTP 404 Not Found
class NotFound : public HttpError
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    NotFound(const std::string &message = "") throw() : HttpError(message) {}
};

// HTTP 409 Conflict
class Conflict : public HttpError
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    Conflict(const std::string &message = "") throw() : HttpError(message) {}
};

// HTTP 5xx Server Error
class ServerError : public HttpError
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    ServerError(int32_t statusCode) throw()
        : HttpError(std::string("HTTP ") + std::to_string(statusCode))
    {}
};

class NetworkError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    NetworkError(const std::string &message = "") throw() : BaseException(message) {}
};

class Canceled : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    Canceled(const std::string &message = "") throw() : BaseException(message) {}
};

class Timeout : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    Timeout(const std::string &message = "") throw() : BaseException(message) {}
};

class ProtocolError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    ProtocolError(const std::string &message = "") throw() : BaseException(message) {}
};

class AddressExists : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    AddressExists(const std::string &message = "") throw() : BaseException(message) {}
};

class AddressBlocked : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    AddressBlocked(const std::string &message = "") throw() : BaseException(message) {}
};

}
}
