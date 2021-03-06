// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from client.djinni

#include "Client.h"  // my header
#include "Address.h"
#include "AsyncTask.h"
#include "ClientAddressExistsListener.h"
#include "ClientCheckCredentialsListener.h"
#include "ClientCreateSessionListener.h"
#include "ClientGenerateKeysListener.h"
#include "MasterKey.h"
#include "SessionListener.h"
#include "jni/support-lib/jni/Marshal.hpp"

namespace JNI { namespace Kullo { namespace Api {

Client::Client() : ::djinni::JniInterface<::Kullo::Api::Client, Client>("net/kullo/libkullo/api/Client$CppProxy") {}

Client::~Client() = default;


CJNIEXPORT void JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::Kullo::Api::Client>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_create(JNIEnv* jniEnv, jobject /*this*/)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::Kullo::Api::Client::create();
        return ::djinni::release(::JNI::Kullo::Api::Client::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_native_1createSessionAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_address, jobject j_masterKey, jstring j_dbFilePath, jobject j_sessionListener, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Client>(nativeRef);
        DJINNI_ASSERT_MSG(j_sessionListener, jniEnv, "Got unexpected null parameter 'sessionListener' to function net.kullo.libkullo.api.Client#createSessionAsync(net.kullo.libkullo.api.Address address, net.kullo.libkullo.api.MasterKey masterKey, String dbFilePath, net.kullo.libkullo.api.SessionListener sessionListener, net.kullo.libkullo.api.ClientCreateSessionListener listener)");
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.Client#createSessionAsync(net.kullo.libkullo.api.Address address, net.kullo.libkullo.api.MasterKey masterKey, String dbFilePath, net.kullo.libkullo.api.SessionListener sessionListener, net.kullo.libkullo.api.ClientCreateSessionListener listener)");
        auto r = ref->createSessionAsync(::JNI::Kullo::Api::Address::toCpp(jniEnv, j_address),
                                         ::JNI::Kullo::Api::MasterKey::toCpp(jniEnv, j_masterKey),
                                         ::djinni::String::toCpp(jniEnv, j_dbFilePath),
                                         ::JNI::Kullo::Api::SessionListener::toCpp(jniEnv, j_sessionListener),
                                         ::JNI::Kullo::Api::ClientCreateSessionListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_native_1addressExistsAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_address, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Client>(nativeRef);
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.Client#addressExistsAsync(net.kullo.libkullo.api.Address address, net.kullo.libkullo.api.ClientAddressExistsListener listener)");
        auto r = ref->addressExistsAsync(::JNI::Kullo::Api::Address::toCpp(jniEnv, j_address),
                                         ::JNI::Kullo::Api::ClientAddressExistsListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_native_1checkCredentialsAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_address, jobject j_masterKey, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Client>(nativeRef);
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.Client#checkCredentialsAsync(net.kullo.libkullo.api.Address address, net.kullo.libkullo.api.MasterKey masterKey, net.kullo.libkullo.api.ClientCheckCredentialsListener listener)");
        auto r = ref->checkCredentialsAsync(::JNI::Kullo::Api::Address::toCpp(jniEnv, j_address),
                                            ::JNI::Kullo::Api::MasterKey::toCpp(jniEnv, j_masterKey),
                                            ::JNI::Kullo::Api::ClientCheckCredentialsListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_native_1generateKeysAsync(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Client>(nativeRef);
        DJINNI_ASSERT_MSG(j_listener, jniEnv, "Got unexpected null parameter 'listener' to function net.kullo.libkullo.api.Client#generateKeysAsync(net.kullo.libkullo.api.ClientGenerateKeysListener listener)");
        auto r = ref->generateKeysAsync(::JNI::Kullo::Api::ClientGenerateKeysListener::toCpp(jniEnv, j_listener));
        return ::djinni::release(::JNI::Kullo::Api::AsyncTask::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT jobject JNICALL Java_net_kullo_libkullo_api_Client_00024CppProxy_native_1versions(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::Kullo::Api::Client>(nativeRef);
        auto r = ref->versions();
        return ::djinni::release(::djinni::Map<::djinni::String, ::djinni::String>::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

} } }  // namespace JNI::Kullo::Api
