// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from messages.djinni

#include "MessageAttachments.h"  // my header
#include "AsyncTask.h"
#include "MessageAttachmentsContentListener.h"
#include "MessageAttachmentsSaveToListener.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

MessageAttachments::MessageAttachments() : ::djinni::JniInterface<::Kullo::Api::MessageAttachments, MessageAttachments>("net/kullo/libkullo/api/MessageAttachments$CppProxy") {}

MessageAttachments::~MessageAttachments() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::Kullo::Api::MessageAttachments>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1allForMessage(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        auto r = ref->allForMessage(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::List<::djinni::I64>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1allAttachmentsDownloaded(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        auto r = ref->allAttachmentsDownloaded(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1filename(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jlong j_attId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        auto r = ref->filename(::djinni::I64::toCpp(jniEnv, j_msgId),
                               ::djinni::I64::toCpp(jniEnv, j_attId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1mimeType(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jlong j_attId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        auto r = ref->mimeType(::djinni::I64::toCpp(jniEnv, j_msgId),
                               ::djinni::I64::toCpp(jniEnv, j_attId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jlong JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1size(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jlong j_attId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        auto r = ref->size(::djinni::I64::toCpp(jniEnv, j_msgId),
                           ::djinni::I64::toCpp(jniEnv, j_attId));
        return ::djinni::release(::djinni::I64::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1hash(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jlong j_attId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        auto r = ref->hash(::djinni::I64::toCpp(jniEnv, j_msgId),
                           ::djinni::I64::toCpp(jniEnv, j_attId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1contentAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jlong j_attId, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.MessageAttachments#contentAsync(long msgId, long attId, net.kullo.libkullo.api.MessageAttachmentsContentListener listener)");
        auto r = ref->contentAsync(::djinni::I64::toCpp(jniEnv, j_msgId),
                                   ::djinni::I64::toCpp(jniEnv, j_attId),
                                   ::JNI::Kullo::Api::MessageAttachmentsContentListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_MessageAttachments_00024CppProxy_native_1saveToAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jlong j_attId, jstring j_path, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::MessageAttachments>(nativeRef);
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.MessageAttachments#saveToAsync(long msgId, long attId, String path, net.kullo.libkullo.api.MessageAttachmentsSaveToListener listener)");
        auto r = ref->saveToAsync(::djinni::I64::toCpp(jniEnv, j_msgId),
                                  ::djinni::I64::toCpp(jniEnv, j_attId),
                                  ::djinni::String::toCpp(jniEnv, j_path),
                                  ::JNI::Kullo::Api::MessageAttachmentsSaveToListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
