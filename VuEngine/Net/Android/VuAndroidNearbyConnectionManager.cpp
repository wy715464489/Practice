//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuNearbyConnectionManager
// 
//*****************************************************************************

#include "VuAndroidNearbyConnectionManager.h"
#include "VuEngine/Events/VuEventManager.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuNearbyConnectionManager, VuAndroidNearbyConnectionManager);

// static JAVA stuff
static JNIEnv		*s_jniEnv;
static jobject		s_helperObject;
static jmethodID	s_startAdvertising;
static jmethodID	s_startDiscovery;
static jmethodID	s_sendConnectionRequest;
static jmethodID	s_reset;
static jmethodID	s_sendMessage;


//*****************************************************************************
void VuAndroidNearbyConnectionManager::bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod)
{
	__android_log_print(ANDROID_LOG_DEBUG, "Nearby",  "VuAndroidNearbyConnectionManager::bindJavaMethods()\n");

	s_jniEnv = jniEnv;

	// get reference to helper class object
	jstring helperClassName = jniEnv->NewStringUTF("com/vectorunit/VuNearbyConnectionHelper");
	jclass helperClass = (jclass)jniEnv->CallObjectMethod(classLoaderObject, findClassMethod, helperClassName);
	jniEnv->DeleteLocalRef(helperClassName);

	jmethodID getInstance = jniEnv->GetStaticMethodID(helperClass, "getInstance", "()Lcom/vectorunit/VuNearbyConnectionHelper;");
	jobject helperObject = jniEnv->CallStaticObjectMethod(helperClass, getInstance);
	s_helperObject = jniEnv->NewGlobalRef(helperObject);

	// methods
	s_startAdvertising = jniEnv->GetMethodID(helperClass, "startAdvertising", "()V");
	s_startDiscovery = jniEnv->GetMethodID(helperClass, "startDiscovery", "()V");
	s_sendConnectionRequest = jniEnv->GetMethodID(helperClass, "sendConnectionRequest", "(Ljava/lang/String;)V");
	s_reset = jniEnv->GetMethodID(helperClass, "reset", "()V");
	s_sendMessage = jniEnv->GetMethodID(helperClass, "sendMessage", "(Ljava/lang/String;[B)V");
}

//*****************************************************************************
bool VuAndroidNearbyConnectionManager::init()
{
	return true;
}

//*****************************************************************************
void VuAndroidNearbyConnectionManager::startAdvertisingNative()
{
	s_jniEnv->CallVoidMethod(s_helperObject, s_startAdvertising);
}

//*****************************************************************************
void VuAndroidNearbyConnectionManager::startDiscoveryNative()
{
	s_jniEnv->CallVoidMethod(s_helperObject, s_startDiscovery);
}

//*****************************************************************************
void VuAndroidNearbyConnectionManager::sendConnectionRequestNative(const char *endpointId)
{
	jstring jEndpointId = s_jniEnv->NewStringUTF(endpointId);
	s_jniEnv->CallVoidMethod(s_helperObject, s_sendConnectionRequest, jEndpointId);
	s_jniEnv->DeleteLocalRef(jEndpointId);
}

//*****************************************************************************
void VuAndroidNearbyConnectionManager::resetNative()
{
	s_jniEnv->CallVoidMethod(s_helperObject, s_reset);
}

//*****************************************************************************
void VuAndroidNearbyConnectionManager::sendMessageNative(const char *endpointId, void *pData, int dataSize)
{
	jstring jEndpointId = s_jniEnv->NewStringUTF(endpointId);
	jbyteArray jPayload = s_jniEnv->NewByteArray(dataSize);

	jbyte *payloadPtr = s_jniEnv->GetByteArrayElements(jPayload, NULL);
	memcpy(payloadPtr, pData, dataSize);
	s_jniEnv->ReleaseByteArrayElements(jPayload, payloadPtr, 0);

	s_jniEnv->CallVoidMethod(s_helperObject, s_sendMessage, jEndpointId, jPayload);

	s_jniEnv->DeleteLocalRef(jEndpointId);
	s_jniEnv->DeleteLocalRef(jPayload);
}


extern "C"
{
	JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeEndpointFound(JNIEnv *env, jobject obj, jstring jEndpointId, jstring jName);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeEndpointLost(JNIEnv *env, jobject obj, jstring jEndpointId);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeConnectionResponse(JNIEnv *env, jobject obj, jstring jEndpointId, jboolean jSuccess);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeConnected(JNIEnv *env, jobject obj, jstring jEndpointId, jstring jName);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeDisconnected(JNIEnv *env, jobject obj, jstring jEndpointId);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeMessageReceived(JNIEnv *env, jobject obj, jstring jEndpointId, jbyteArray jPayload);
}


//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeEndpointFound(JNIEnv *env, jobject obj, jstring jEndpointId, jstring jName)
{
	const char *strEndpointId = env->GetStringUTFChars(jEndpointId, 0);
	const char *strName = env->GetStringUTFChars(jName, 0);

	VuParams params;
	params.addString(strEndpointId);
	params.addString(strName);
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnNearbyConnectionEndpointFound", params);

	env->ReleaseStringUTFChars(jEndpointId, strEndpointId);
	env->ReleaseStringUTFChars(jName, strName);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeEndpointLost(JNIEnv *env, jobject obj, jstring jEndpointId)
{
	const char *strEndpointId = env->GetStringUTFChars(jEndpointId, 0);

	VuParams params;
	params.addString(strEndpointId);
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnNearbyConnectionEndpointLost", params);

	env->ReleaseStringUTFChars(jEndpointId, strEndpointId);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeConnectionResponse(JNIEnv *env, jobject obj, jstring jEndpointId, jboolean jSuccess)
{
	const char *strEndpointId = env->GetStringUTFChars(jEndpointId, 0);

	VuParams params;
	params.addString(strEndpointId);
	params.addBool(jSuccess);
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnNearbyConnectionConnectionResponse", params);

	env->ReleaseStringUTFChars(jEndpointId, strEndpointId);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeConnected(JNIEnv *env, jobject obj, jstring jEndpointId, jstring jName)
{
	const char *strEndpointId = env->GetStringUTFChars(jEndpointId, 0);
	const char *strName = env->GetStringUTFChars(jName, 0);

	VuParams params;
	params.addString(strEndpointId);
	params.addString(strName);
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnNearbyConnectionConnected", params);

	env->ReleaseStringUTFChars(jEndpointId, strEndpointId);
	env->ReleaseStringUTFChars(jName, strName);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeDisconnected(JNIEnv *env, jobject obj, jstring jEndpointId)
{
	const char *strEndpointId = env->GetStringUTFChars(jEndpointId, 0);

	VuParams params;
	params.addString(strEndpointId);
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnNearbyConnectionDisconnected", params);

	env->ReleaseStringUTFChars(jEndpointId, strEndpointId);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuNearbyConnectionHelper_nativeMessageReceived(JNIEnv *env, jobject obj, jstring jEndpointId, jbyteArray jPayload)
{
	const char *strEndpointId = env->GetStringUTFChars(jEndpointId, 0);
	jbyte *payloadPtr = env->GetByteArrayElements(jPayload, NULL);
	int payloadSize = env->GetArrayLength(jPayload);

	void *payloadCopy = malloc(payloadSize);
	VU_MEMCPY(payloadCopy, payloadSize, payloadPtr, payloadSize);

	VuParams params;
	params.addString(strEndpointId);
	params.addPointer(payloadCopy);
	params.addInt(payloadSize);
	params.addBool(true);

	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnNearbyConnectionMessageReceived", params);

	env->ReleaseByteArrayElements(jPayload, payloadPtr, 0);
	env->ReleaseStringUTFChars(jEndpointId, strEndpointId);
}
