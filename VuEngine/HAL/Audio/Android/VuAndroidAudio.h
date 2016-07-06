//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android Audio HAL
//
//*****************************************************************************

#pragma once

#include <jni.h>
#include "VuEngine/HAL/Audio/VuAudio.h"


class VuAndroidAudio : public VuAudio
{
public:
	// this function MUST be called from the application's JNI_OnLoad,
	// from a function known to be called by JNI_OnLoad, or from a function
	// in a Java-called thread.
	static void			bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod);

	virtual bool		isDolbyAudioProcessingSupported();
	virtual bool		isDolbyAudioProcessingEnabled();
	virtual void		setDolbyAudioProcessingEnabled(bool enable);
};
