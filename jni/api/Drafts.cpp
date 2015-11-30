// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#include "Drafts.h"  // my header
#include "DraftState.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

Drafts::Drafts() : ::djinni::JniInterface<::Kullo::Api::Drafts, Drafts>("net/kullo/libkullo/api/Drafts$CppProxy") {}

Drafts::~Drafts() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Drafts_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<djinni::CppProxyHandle<::Kullo::Api::Drafts>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Drafts_00024CppProxy_native_1text(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_convId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Drafts>::get(nativeRef);
        auto r = ref->text(::djinni::I64::toCpp(jniEnv, j_convId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Drafts_00024CppProxy_native_1setText(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_convId, jstring j_text)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Drafts>::get(nativeRef);
        ref->setText(::djinni::I64::toCpp(jniEnv, j_convId),
                     ::djinni::String::toCpp(jniEnv, j_text));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Drafts_00024CppProxy_native_1state(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_convId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Drafts>::get(nativeRef);
        auto r = ref->state(::djinni::I64::toCpp(jniEnv, j_convId));
        return ::djinni::release(::JNI::Kullo::Api::DraftState::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Drafts_00024CppProxy_native_1prepareToSend(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_convId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Drafts>::get(nativeRef);
        ref->prepareToSend(::djinni::I64::toCpp(jniEnv, j_convId));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Drafts_00024CppProxy_native_1clear(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_convId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Drafts>::get(nativeRef);
        ref->clear(::djinni::I64::toCpp(jniEnv, j_convId));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

} } }  // namespace JNI::Kullo::Api
