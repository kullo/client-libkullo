// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from api.djinni

#include "Messages.h"  // my header
#include "Address.h"
#include "DateTime.h"
#include "Delivery.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

Messages::Messages() : ::djinni::JniInterface<::Kullo::Api::Messages, Messages>("net/kullo/libkullo/api/Messages$CppProxy") {}

Messages::~Messages() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<djinni::CppProxyHandle<::Kullo::Api::Messages>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1allForConversation(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_convId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->allForConversation(::djinni::I64::toCpp(jniEnv, j_convId));
        return ::djinni::release(::djinni::List<::djinni::I64>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jlong JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1latestForSender(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_address)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->latestForSender(::JNI::Kullo::Api::Address::toCpp(jniEnv, j_address));
        return ::djinni::release(::djinni::I64::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1remove(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        ref->remove(::djinni::I64::toCpp(jniEnv, j_msgId));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jlong JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1conversation(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->conversation(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::I64::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1deliveryState(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->deliveryState(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::List<::JNI::Kullo::Api::Delivery>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1isRead(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->isRead(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1setRead(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jboolean j_value)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        ref->setRead(::djinni::I64::toCpp(jniEnv, j_msgId),
                     ::djinni::Bool::toCpp(jniEnv, j_value));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jboolean JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1isDone(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->isDone(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::Bool::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1setDone(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId, jboolean j_value)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        ref->setDone(::djinni::I64::toCpp(jniEnv, j_msgId),
                     ::djinni::Bool::toCpp(jniEnv, j_value));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1dateSent(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->dateSent(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::JNI::Kullo::Api::DateTime::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1dateReceived(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->dateReceived(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::JNI::Kullo::Api::DateTime::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1text(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->text(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1textAsHtml(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->textAsHtml(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jstring JNICALL Java_net_kullo_libkullo_api_Messages_00024CppProxy_native_1footer(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jlong j_msgId)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::CppProxyHandle<::Kullo::Api::Messages>::get(nativeRef);
        auto r = ref->footer(::djinni::I64::toCpp(jniEnv, j_msgId));
        return ::djinni::release(::djinni::String::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
