#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>

#include <string>

std::string osVersionAppleImpl()
{
    NSProcessInfo *processInfo = [[NSProcessInfo alloc] init];

    // check avaiability of the property operatingSystemVersion (10.10+) at runtime
    if ([processInfo respondsToSelector:@selector(operatingSystemVersion)])
    {
        NSOperatingSystemVersion versionObj = [processInfo operatingSystemVersion];
        return std::to_string(versionObj.majorVersion) + "." + std::to_string(versionObj.minorVersion);
    }
    else
    {
        return "10.9";
    }
}
