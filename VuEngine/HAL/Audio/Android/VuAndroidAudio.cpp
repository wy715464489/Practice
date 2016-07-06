//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android Audio HAL
//
//*****************************************************************************

#include "VuAndroidAudio.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAudio, VuAndroidAudio);


// static JAVA stuff
static JNIEnv		*s_jniEnv;
static jobject		s_helperObject;
static jmethodID	s_isDolbyAudioProcessingSupported;
static jmethodID	s_isDolbyAudioProcessingEnabled;
static jmethodID	s_setDolbyAudioProcessingEnabled;


//*****************************************************************************
void VuAndroidAudio::bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod)
{
	__android_log_print(ANDROID_LOG_DEBUG, "Audio",  "VuAndroidAudio::bindJavaMethods()\n");

	s_jniEnv = jniEnv;

	// get reference to helper class object
	jstring helperClassName = jniEnv->NewStringUTF("com/vectorunit/VuAudioHelper");
	jclass helperClass = (jclass)jniEnv->CallObjectMethod(classLoaderObject, findClassMethod, helperClassName);
	jniEnv->DeleteLocalRef(helperClassName);

	jmethodID getInstance = jniEnv->GetStaticMethodID(helperClass, "getInstance", "()Lcom/vectorunit/VuAudioHelper;");
	jobject helperObject = jniEnv->CallStaticObjectMethod(helperClass, getInstance);
	s_helperObject = jniEnv->NewGlobalRef(helperObject);

	// methods
	s_isDolbyAudioProcessingSupported = jniEnv->GetMethodID(helperClass, "isDolbyAudioProcessingSupported", "()Z");
	s_isDolbyAudioProcessingEnabled = jniEnv->GetMethodID(helperClass, "isDolbyAudioProcessingEnabled", "()Z");
	s_setDolbyAudioProcessingEnabled = jniEnv->GetMethodID(helperClass, "setDolbyAudioProcessingEnabled", "(Z)V");
}

//*****************************************************************************
bool VuAndroidAudio::isDolbyAudioProcessingSupported()
{
	return s_jniEnv->CallBooleanMethod(s_helperObject, s_isDolbyAudioProcessingSupported);
}

//*****************************************************************************
bool VuAndroidAudio::isDolbyAudioProcessingEnabled()
{
	return s_jniEnv->CallBooleanMethod(s_helperObject, s_isDolbyAudioProcessingEnabled);
}

//*****************************************************************************
void VuAndroidAudio::setDolbyAudioProcessingEnabled(bool enable)
{
	s_jniEnv->CallBooleanMethod(s_helperObject, s_setDolbyAudioProcessingEnabled, enable);
}
