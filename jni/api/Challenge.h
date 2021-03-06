// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from registration.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/Challenge.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class Challenge final : ::djinni::JniInterface<::Kullo::Api::Challenge, Challenge> {
public:
    using CppType = ::Kullo::nn_shared_ptr<::Kullo::Api::Challenge>;
    using CppOptType = std::shared_ptr<::Kullo::Api::Challenge>;
    using JniType = jobject;

    using Boxed = Challenge;

    ~Challenge();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
        DJINNI_ASSERT_MSG(j, jniEnv, "Challenge::fromCpp requires a non-null Java object");
        return kulloForcedNn(::djinni::JniClass<Challenge>::get()._fromJava(jniEnv, j));
    };
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<Challenge>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    Challenge();
    friend ::djinni::JniClass<Challenge>;
    friend ::djinni::JniInterface<::Kullo::Api::Challenge, Challenge>;

};

} } }  // namespace JNI::Kullo::Api
