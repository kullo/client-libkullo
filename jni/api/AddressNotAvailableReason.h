// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from registration.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/AddressNotAvailableReason.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class AddressNotAvailableReason final : ::djinni::JniEnum {
public:
    using CppType = ::Kullo::Api::AddressNotAvailableReason;
    using JniType = jobject;

    using Boxed = AddressNotAvailableReason;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<AddressNotAvailableReason>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<AddressNotAvailableReason>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    AddressNotAvailableReason() : JniEnum("net/kullo/libkullo/api/AddressNotAvailableReason") {}
    friend ::djinni::JniClass<AddressNotAvailableReason>;
};

} } }  // namespace JNI::Kullo::Api
