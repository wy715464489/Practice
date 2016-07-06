//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android Http Client
//
//*****************************************************************************

#pragma once

#include <jni.h>
#include "VuEngine/Net/VuHttpClient.h"


class VuAndroidHttpClient : public VuHttpClient
{
public:
	// this function MUST be called from the application's JNI_OnLoad,
	// from a function known to be called by JNI_OnLoad, or from a function
	// in a Java-called thread.
	static void			bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod);

	// public interface
	virtual VUHANDLE	createRequest();
	virtual void		releaseRequest(VUHANDLE hRequest);
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, const char *value);
	virtual void		setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS);

	virtual void		sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size);

	virtual eStatus		getStatus(VUHANDLE hRequest);

	virtual const std::string	&getResponse(VUHANDLE hRequest);

	//// platform-specific functionality
	//static VuAndroidHttpClient *IF() { return static_cast<VuAndroidHttpClient *>(VuHttpClient::IF()); }
};
