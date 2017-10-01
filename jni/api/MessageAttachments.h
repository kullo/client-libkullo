// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from messages.djinni

#pragma once

#include "jni/support-lib/jni/djinni_support.hpp"
#include "kulloclient/api/MessageAttachments.h"
#include <kulloclient/nn.h>

namespace JNI { namespace Kullo { namespace Api {

class MessageAttachments final : ::djinni::JniInterface<::Kullo::Api::MessageAttachments, MessageAttachments> {
public:
    using CppType = ::Kullo::nn_shared_ptr<::Kullo::Api::MessageAttachments>;
    using CppOptType = std::shared_ptr<::Kullo::Api::MessageAttachments>;
    using JniType = jobject;

    using Boxed = MessageAttachments;

    ~MessageAttachments();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
        DJINNI_ASSERT_MSG(j, jniEnv, "MessageAttachments::fromCpp requires a non-null Java object");
        return kulloForcedNn(::djinni::JniClass<MessageAttachments>::get()._fromJava(jniEnv, j));
    };
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<MessageAttachments>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    MessageAttachments();
    friend ::djinni::JniClass<MessageAttachments>;
    friend ::djinni::JniInterface<::Kullo::Api::MessageAttachments, MessageAttachments>;

};

} } }  // namespace JNI::Kullo::Api
