//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuNearbyConnectionManager
// 
//*****************************************************************************

#pragma once

#include <jni.h>
#include "VuEngine/Net/VuNearbyConnectionManager.h"


class VuAndroidNearbyConnectionManager : public VuNearbyConnectionManager
{
public:
	// this function MUST be called from the application's JNI_OnLoad,
	// from a function known to be called by JNI_OnLoad, or from a function
	// in a Java-called thread.
	static void		bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod);

	virtual bool	init();

	virtual void	startAdvertisingNative();
	virtual void	startDiscoveryNative();
	virtual void	sendConnectionRequestNative(const char *endpointId);
	virtual void	resetNative();
	virtual void	sendMessageNative(const char *endpointId, void *pData, int dataSize);
};

