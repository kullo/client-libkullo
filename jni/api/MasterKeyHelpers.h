// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from masterkey.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/MasterKeyHelpers.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class MasterKeyHelpers final : ::djinni::JniInterface<::Kullo::Api::MasterKeyHelpers, MasterKeyHelpers> {
public:
    using CppType = ::Kullo::nn_shared_ptr<::Kullo::Api::MasterKeyHelpers>;
    using CppOptType = std::shared_ptr<::Kullo::Api::MasterKeyHelpers>;
    using JniType = jobject;

    using Boxed = MasterKeyHelpers;

    ~MasterKeyHelpers();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
        DJINNI_ASSERT_MSG(j, jniEnv, "MasterKeyHelpers::fromCpp requires a non-null Java object");
        return kulloForcedNn(::djinni::JniClass<MasterKeyHelpers>::get()._fromJava(jniEnv, j));
    };
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<MasterKeyHelpers>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    MasterKeyHelpers();
    friend ::djinni::JniClass<MasterKeyHelpers>;
    friend ::djinni::JniInterface<::Kullo::Api::MasterKeyHelpers, MasterKeyHelpers>;

};

} } }  // namespace JNI::Kullo::Api