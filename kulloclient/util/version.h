/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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

    /// Returns the version of iOS and macOS and crashes when used on other systems
    static std::string osVersionApple();
};

}
}
