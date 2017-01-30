/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include "kulloclient/util/exceptions.h"

namespace Kullo {
namespace Codec {

class InvalidContentFormat : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    InvalidContentFormat(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~InvalidContentFormat() = default;
};

class DecryptionKeyMissing : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    DecryptionKeyMissing(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~DecryptionKeyMissing() = default;
};

class UnsupportedContentVersion : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    UnsupportedContentVersion(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~UnsupportedContentVersion() = default;
};

class SignatureVerificationKeyMissing : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    SignatureVerificationKeyMissing(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~SignatureVerificationKeyMissing() = default;
};

class SignatureVerificationFailed : public Util::BaseException
{
public:
    /// @copydoc BaseException::BaseException(const std::string&)
    SignatureVerificationFailed(const std::string &message = "") throw() : BaseException(message) {}
    virtual ~SignatureVerificationFailed() = default;
};

}
}
