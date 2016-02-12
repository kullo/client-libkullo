// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from push.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/PushTokenEnvironment.h"

namespace JNI { namespace Kullo { namespace Api {

class PushTokenEnvironment final : ::djinni::JniEnum {
public:
    using CppType = ::Kullo::Api::PushTokenEnvironment;
    using JniType = jobject;

    using Boxed = PushTokenEnvironment;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<PushTokenEnvironment>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<PushTokenEnvironment>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    PushTokenEnvironment() : JniEnum("net/kullo/libkullo/api/PushTokenEnvironment") {}
    friend ::djinni::JniClass<PushTokenEnvironment>;
};

} } }  // namespace JNI::Kullo::Api
