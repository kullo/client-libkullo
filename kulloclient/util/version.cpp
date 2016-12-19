/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/version.h"

#include <boost/version.hpp>
#include <botan/botan_all.h>
#include <jsoncpp/jsoncpp.h>
#include <smartsqlite/version.h>

namespace Kullo {
namespace Util {

namespace {

#if defined __ANDROID__
    const std::string KULLO_OS = "Android";
    const std::string KULLO_OS_USERVERSION = "unknown";
#elif defined __linux__
    #if defined __LP64__
        const std::string KULLO_OS = "Linux_x86_64";
        const std::string KULLO_OS_USERVERSION = "unknown";
    #else
        const std::string KULLO_OS = "Linux_x86_32";
        const std::string KULLO_OS_USERVERSION = "unknown";
    #endif
#elif defined _WIN32 // is also defined on 64 bit
    const std::string KULLO_OS = "Windows";
    const std::string KULLO_OS_USERVERSION = "unknown";
#elif defined __APPLE__ && __MACH__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        const std::string KULLO_OS = "iOS";
        const std::string KULLO_OS_USERVERSION = "unknown";
    #else
        const std::string KULLO_OS = "OSX";

        #include <CoreFoundation/CoreFoundation.h>
        std::string getOsxVersion() {
            std::string out;
            if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_11) {
                out = "10.11+";
            } else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_10) {
                out = "10.10";
            } else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_9) {
                out = "10.9";
            } else {
                out = "unknown";
            }
            return out;
        }

        const std::string KULLO_OS_USERVERSION = getOsxVersion();
    #endif
#else
    #error unknown platform
#endif

const std::string LIBKULLO_VERSION = "v59";
const std::string LIBKULLO_USER_AGENT =
        "libkulloclient/" + LIBKULLO_VERSION + " (" + KULLO_OS + "/" + KULLO_OS_USERVERSION + ")";
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
