// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from masterkey.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/InternalMasterKeyUtils.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class InternalMasterKeyUtils final : ::djinni::JniInterface<::Kullo::Api::InternalMasterKeyUtils, InternalMasterKeyUtils> {
public:
    using CppType = ::Kullo::nn_shared_ptr<::Kullo::Api::InternalMasterKeyUtils>;
    using CppOptType = std::shared_ptr<::Kullo::Api::InternalMasterKeyUtils>;
    using JniType = jobject;

    using Boxed = InternalMasterKeyUtils;

    ~InternalMasterKeyUtils();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
        DJINNI_ASSERT_MSG(j, jniEnv, "InternalMasterKeyUtils::fromCpp requires a non-null Java object");
        return kulloForcedNn(::djinni::JniClass<InternalMasterKeyUtils>::get()._fromJava(jniEnv, j));
    };
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<InternalMasterKeyUtils>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    InternalMasterKeyUtils();
    friend ::djinni::JniClass<InternalMasterKeyUtils>;
    friend ::djinni::JniInterface<::Kullo::Api::InternalMasterKeyUtils, InternalMasterKeyUtils>;

};

} } }  // namespace JNI::Kullo::Api
