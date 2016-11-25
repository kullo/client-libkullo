/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <exception>
#include <string>

#include "kulloclient/util/nested_exception.h"

#define FWD_NESTED(statement, catchType, nestedException) \
    try {                                         \
        statement;                                \
    } catch(catchType) {                          \
        std::throw_with_nested(nestedException);  \
    }

namespace Kullo {
namespace Util {

std::string formatException(const std::exception &ex, size_t indent = 0);

/// Base class of all exceptions defined in this file.
class BaseException : public std::exception
{
public:
    /// Creates a new instance with the given message.
    BaseException(const std::string &message = "") throw();
    virtual ~BaseException() = default;

    /**
     * @brief Returns the textual representation of this exception.
     *
     * Override this to customize exception messages.
     */
    virtual const char* what() const throw();

protected:
    /// the message that is returned by the base implementation of what()
    std::string message_;
};

/// An exception for errors while accessing the file system.
class FilesystemError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    FilesystemError(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~FilesystemError() = default;
};

/// An exception for errors when a file is too big.
class FileTooBigError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    FileTooBigError(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~FileTooBigError() = default;
};

/// An exception for errors while decompressing Gzip data.
class GZipStreamError : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    GZipStreamError(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~GZipStreamError() = default;
};

/// An exception for invalud agument values.
class InvalidArgument : public Util::BaseException
{
public:
    /// @copydoc InvalidArgument::InvalidArgument(const std::string&)
    InvalidArgument(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~InvalidArgument() = default;
};

/// An exception for errors while constructing a DateTime
class InvalidDateTime : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    InvalidDateTime(const std::string &message) throw() : BaseException(message) {}
    virtual ~InvalidDateTime() = default;
};

/// An exception for errors while constructing a DateTime
class EmptyDateTime : public InvalidDateTime
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    EmptyDateTime() throw()
        : InvalidDateTime("Tried to create DateTime from empty string") {}
    virtual ~EmptyDateTime() = default;
};

}
}
