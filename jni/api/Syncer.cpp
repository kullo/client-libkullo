// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from syncer.djinni

#include "Syncer.h"  // my header
#include "DateTime.h"
#include "SyncMode.h"
#include "SyncerListener.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

Syncer::Syncer() : ::djinni::JniInterface<::Kullo::Api::Syncer, Syncer>("net/kullo/libkullo/api/Syncer$CppProxy") {}

Syncer::~Syncer() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<djinni::CppProxyHandle<::Kullo::Api::Syncer>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1setListener(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        ref->setListener(::JNI::Kullo::Api::SyncerListener::toCpp(jniEnv, j_listener));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1lastFullSync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        auto r = ref->lastFullSync();
        return ::djinni::release(::djinni::Optional<boost::optional, ::JNI::Kullo::Api::DateTime>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1requestSync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_mode)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        ref->requestSync(::JNI::Kullo::Api::SyncMode::toCpp(jniEnv, j_mode));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1requestDownloadingAttachmentsForMessage(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        ref->requestDownloadingAttachmentsForMessage(::djinni::I64::toCpp(jniEnv, j_msgId));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1cancel(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        ref->cancel();
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1isSyncing(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        auto r = ref->isSyncing();
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1waitUntilDone(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        ref->waitUntilDone();
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_Syncer_00024CppProxy_native_1waitForMs(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jint j_timeout)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Syncer>(nativeRef);
        auto r = ref->waitForMs(::djinni::I32::toCpp(jniEnv, j_timeout));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
