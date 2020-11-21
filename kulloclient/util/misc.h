/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#define K_UNUSED(x) (void)x
#define K_RAII(x) (void)x

#ifdef _MSC_VER
    #define K_NORETURN __declspec(noreturn)
#else
    #define K_NORETURN __attribute__((noreturn))
#endif

#define K_DISABLE_COPY(ClassName) \
    ClassName(const ClassName &) = delete; \
    ClassName &operator=(const ClassName &) = delete

#define K_AS_STRING(x) std::string(#x)

#include <memory>

namespace Kullo {
    // Have Kullo::make_unique avalable on all supported platforms.
    // This way, we don't need to write into someone else's namespace.
    //
    // Fallback implementation from
    // http://herbsutter.com/gotw/_102/
    //
    // In MSVC13 std::make_unique is available in <memory>
    // http://msdn.microsoft.com/de-de/library/ee410601.aspx
    // but it does not set the feature test __cpp_lib_make_uniques
    // http://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations#recs.cpp14
#if defined _MSC_VER || defined __cpp_lib_make_unique
    using std::make_unique;
#else
    template<typename T, typename ...Args>
    std::unique_ptr<T> make_unique(Args&& ...args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#endif
}
