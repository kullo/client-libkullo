/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <string>

namespace Kullo {
namespace Util {

/// Provides access to the library's version information
class Version
{
public:
    /// Returns the user agent of the HTTP client, which contains the platform and the client version.
    static std::string userAgent();

    /// Returns the libkullo version.
    static std::string libkulloVersion();

    /// Returns the boost version.
    static std::string boostVersion();

    /// Returns the botan version.
    static std::string botanVersion();

    /// Returns the JsonCpp version.
    static std::string jsoncppVersion();

    /// Returns the SmartSqlite version.
    static std::string smartSqliteVersion();

    /// Returns the SQLite version.
    static std::string sqliteVersion();
};

}
}
