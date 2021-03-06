// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from http.djinni

#include "ResponseListener.h"  // my header
#include "ProgressResult.h"
#include "TransferProgress.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Http {

ResponseListener::ResponseListener() : ::djinni::JniInterface<::Kullo::Http::ResponseListener, ResponseListener>("net/kullo/libkullo/http/ResponseListener$CppProxy") {}

ResponseListener::~ResponseListener() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_http_ResponseListener_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::Kullo::Http::ResponseListener>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_http_ResponseListener_00024CppProxy_native_1progressed(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_progress)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Http::ResponseListener>(nativeRef);
        auto r = ref->progressed(::JNI::Kullo::Http::TransferProgress::toCpp(jniEnv, j_progress));
        return ::djinni::release(::JNI::Kullo::Http::ProgressResult::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_http_ResponseListener_00024CppProxy_native_1dataReceived(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jbyteArray j_data)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Http::ResponseListener>(nativeRef);
        ref->dataReceived(::djinni::Binary::toCpp(jniEnv, j_data));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

} } }  // namespace JNI::Kullo::Http
