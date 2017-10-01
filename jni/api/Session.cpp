// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from session.djinni

#include "Session.h"  // my header
#include "AsyncTask.h"
#include "Conversations.h"
#include "DraftAttachments.h"
#include "Drafts.h"
#include "Event.h"
#include "InternalEvent.h"
#include "MessageAttachments.h"
#include "Messages.h"
#include "PushToken.h"
#include "Senders.h"
#include "SessionAccountInfoListener.h"
#include "Syncer.h"
#include "UserSettings.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

Session::Session() : ::djinni::JniInterface<::Kullo::Api::Session, Session>("net/kullo/libkullo/api/Session$CppProxy") {}

Session::~Session() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::Kullo::Api::Session>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1userSettings(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->userSettings();
        return ::djinni::release(::JNI::Kullo::Api::UserSettings::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1conversations(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->conversations();
        return ::djinni::release(::JNI::Kullo::Api::Conversations::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1messages(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->messages();
        return ::djinni::release(::JNI::Kullo::Api::Messages::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1messageAttachments(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->messageAttachments();
        return ::djinni::release(::JNI::Kullo::Api::MessageAttachments::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1senders(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->senders();
        return ::djinni::release(::JNI::Kullo::Api::Senders::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1drafts(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->drafts();
        return ::djinni::release(::JNI::Kullo::Api::Drafts::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1draftAttachments(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->draftAttachments();
        return ::djinni::release(::JNI::Kullo::Api::DraftAttachments::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1syncer(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->syncer();
        return ::djinni::release(::JNI::Kullo::Api::Syncer::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1accountInfoAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.Session#accountInfoAsync(net.kullo.libkullo.api.SessionAccountInfoListener listener)");
        auto r = ref->accountInfoAsync(::JNI::Kullo::Api::SessionAccountInfoListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1registerPushToken(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_token)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->registerPushToken(::JNI::Kullo::Api::PushToken::toCpp(jniEnv, j_token));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1unregisterPushToken(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_token)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        auto r = ref->unregisterPushToken(::JNI::Kullo::Api::PushToken::toCpp(jniEnv, j_token));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Session_00024CppProxy_native_1notify(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_internalEvent)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Session>(nativeRef);
        DJINNI_ASSERT_MSG(j_internalEvent, jniEnv, "Got unexpected null parameter 'internalEvent' to function net.kullo.libkullo.api.Session#notify(net.kullo.libkullo.api.InternalEvent internalEvent)");
        auto r = ref->notify(::JNI::Kullo::Api::InternalEvent::toCpp(jniEnv, j_internalEvent));
        return ::djinni::release(::djinni::List<::JNI::Kullo::Api::Event>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
