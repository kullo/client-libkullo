// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/http/ProgressResult.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Http {

class ProgressResult final : ::djinni::JniEnum {
public:
    using CppType = ::Kullo::Http::ProgressResult;
    using JniType = jobject;

    using Boxed = ProgressResult;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<ProgressResult>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<ProgressResult>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    ProgressResult() : JniEnum("net/kullo/libkullo/http/ProgressResult") {}
    friend ::djinni::JniClass<ProgressResult>;
};

} } }  // namespace JNI::Kullo::Http
