/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/version.h"

#include <boost/version.hpp>
#include <botan/botan.h>
#include <jsoncpp/jsoncpp.h>
#include <smartsqlite/version.h>

#if defined __ANDROID__
    #define KULLO_OS "Android"
#elif defined __linux__
    #if defined __LP64__
        #define KULLO_OS "Linux_x86_64"
    #else
        #define KULLO_OS "Linux_x86_32"
    #endif
#elif defined _WIN32 // is also defined on 64 bit
    #define KULLO_OS "Windows"
#elif defined __APPLE__ && __MACH__
    #include "TargetConditionals.h"
    #if defined TARGET_OS_IPHONE
        #define KULLO_OS "iOS"
    #else
        #define KULLO_OS "OSX"
    #endif
#else
    #error unknown platform
#endif

namespace Kullo {
namespace Util {

namespace {
const std::string LIBKULLO_VERSION = "v43";
const std::string LIBKULLO_USER_AGENT = "libkulloclient/" + LIBKULLO_VERSION + " (" KULLO_OS ")";
}

std::string Version::userAgent()
{
    return LIBKULLO_USER_AGENT;
}

std::string Version::libkulloVersion()
{
    return LIBKULLO_VERSION;
}

std::string Version::boostVersion()
{
    // From boost/version.hpp:
    //   BOOST_VERSION % 100 is the patch level
    //   BOOST_VERSION / 100 % 1000 is the minor version
    //   BOOST_VERSION / 100000 is the major version
    const int boost_major = BOOST_VERSION / 100000;
    const int boost_minor = BOOST_VERSION / 100 % 1000;
    const int boost_patch = BOOST_VERSION % 100;

    return std::to_string(boost_major) + "."
         + std::to_string(boost_minor) + "."
         + std::to_string(boost_patch);
}

std::string Version::botanVersion()
{
    return std::to_string(BOTAN_VERSION_MAJOR) + "."
         + std::to_string(BOTAN_VERSION_MINOR) + "."
         + std::to_string(BOTAN_VERSION_PATCH);
}

std::string Version::jsoncppVersion()
{
    return JSONCPP_VERSION_STRING;
}

std::string Version::smartSqliteVersion()
{
    return SmartSqlite::version();
}

std::string Version::sqliteVersion()
{
    return SmartSqlite::sqliteVersion();
}

}
}
