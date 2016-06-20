#pragma once

// IMPORTANT: This tree must be kept in sync with botan_all.cpp
#ifdef _WIN32
    #ifdef _WIN64
        #error "Unsupported platform"
    #else
        #ifdef _MSC_VER
            #include "win32-msvc/botan_all.h"
        #else
            #error "Unsupported platform"
        #endif
    #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        #if TARGET_CPU_X86
            #include "ios-x86/botan_all.h"
        #elif TARGET_CPU_X86_64
            #include "ios-x86_64/botan_all.h"
        #elif TARGET_CPU_ARM
            #include "ios-armv7/botan_all.h"
        #elif TARGET_CPU_ARM64
            #include "ios-armv8/botan_all.h"
        #else
            #error "Unsupported platform"
        #endif
    #elif TARGET_OS_MAC
        #include "osx/botan_all.h"
    #else
        #error "Unsupported platform"
    #endif
#elif __linux
    #ifdef __ANDROID__
        #ifdef __i386__
            #include "android-x86/botan_all.h"
        #elif defined __arm__
            #include "android-arm/botan_all.h"
        #else
            #error "Unsupported platform"
        #endif
    #else
        #ifdef __LP64__
            #include "linux64/botan_all.h"
        #else
            #include "linux32/botan_all.h"
        #endif
    #endif
#else
    #error "Unsupported platform"
#endif
