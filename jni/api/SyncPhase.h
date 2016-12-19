// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from syncer.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/SyncPhase.h"

namespace JNI { namespace Kullo { namespace Api {

class SyncPhase final : ::djinni::JniEnum {
public:
    using CppType = ::Kullo::Api::SyncPhase;
    using JniType = jobject;

    using Boxed = SyncPhase;

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return static_cast<CppType>(::djinni::JniClass<SyncPhase>::get().ordinal(jniEnv, j)); }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) { return ::djinni::JniClass<SyncPhase>::get().create(jniEnv, static_cast<jint>(c)); }

private:
    SyncPhase() : JniEnum("net/kullo/libkullo/api/SyncPhase") {}
    friend ::djinni::JniClass<SyncPhase>;
};

} } }  // namespace JNI::Kullo::Api