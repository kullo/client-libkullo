// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/NetworkError.h"

namespace JNI { namespace Kullo { namespace Api {

class NetworkError final : ::djinni::JniEnum {
public:
    using CppType = ::Kullo::Api::NetworkError;
    using JniType = jobject;

    using Boxed = NetworkError;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<NetworkError>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<NetworkError>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    NetworkError() : JniEnum("net/kullo/libkullo/api/NetworkError") {}
    friend ::djinni::JniClass<NetworkError>;
};

} } }  // namespace JNI::Kullo::Api
