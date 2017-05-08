/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/version.h"

#include <boost/version.hpp>
#include <botan/botan_all.h>
#include <jsoncpp/jsoncpp.h>
#include <smartsqlite/version.h>

#include "kulloclient/util/assert.h"

// defined in .mm file
extern std::string osVersionAppleImpl();

namespace Kullo {
namespace Util {

namespace {

#if defined __ANDROID__
    const std::string KULLO_OS = "Android";

    #ifndef __ANDROID_API__
        #error No value for __ANDROID_API__ defined. This should come from the toolchain.
    #endif

    #if __ANDROID_API__ < 21
    #include <sys/system_properties.h>
    #endif

    int getSdkVersion() {
        std::string sdkVersionString;

        #if __ANDROID_API__ < 21

        char sdkVersionStringC[PROP_VALUE_MAX];
        __system_property_get("ro.build.version.sdk", sdkVersionStringC);
        sdkVersionString = std::string(sdkVersionStringC);

        #else

        FILE* file = popen("getprop ro.build.version.sdk", "r");
        if (!file) return -1;

        int c;
        while ((c = getc(file)) != EOF)
        {
            sdkVersionString += static_cast<char>(c);
        }
        pclose(file);

        #endif

        try
        {
            return std::stoi(sdkVersionString);
        }
        catch (std::exception)
        {
            return -1;
        }
    }

    const int ANDROID_SDK_VERSION = getSdkVersion();
    const std::string KULLO_OS_USERVERSION = ANDROID_SDK_VERSION == -1
            ? "unknown"
            : std::to_string(ANDROID_SDK_VERSION);

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
        const std::string KULLO_OS_USERVERSION = Util::Version::osVersionApple();
    #else
        const std::string KULLO_OS = "OSX";
        const std::string KULLO_OS_USERVERSION = Util::Version::osVersionApple();
    #endif
#else
    #error unknown platform
#endif

const std::string LIBKULLO_VERSION = "v63";
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

std::string Version::osVersionApple()
{
#ifdef __APPLE__
    return osVersionAppleImpl();
#else
    kulloAssertionFailed("Method must not be called on non-apple systems");
#endif
}

}
}
