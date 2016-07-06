//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android Http Client
//
//*****************************************************************************

#include "VuAndroidHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuAndroidHttpClient);


class VuAndroidHttpRequest : public VuRefObj
{
public:
	VuAndroidHttpRequest() : mStatus(VuHttpClient::STATUS_READY), mTimeoutMS(10000), mConnection(VUNULL) {}

	typedef std::map<std::string, std::string> Header;

	std::string				mURL;
	Header					mHeader;
	std::string				mData;
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;
	int						mTimeoutMS;
	jobject					mConnection;
};


// static JAVA stuff
static JNIEnv		*s_jniEnv;
static jobject		s_helperObject;
static jmethodID	s_openConnection;
static jmethodID	s_setRequestProperty;
static jmethodID	s_setTimeoutMS;
static jmethodID	s_sendRequest;


//*****************************************************************************
void VuAndroidHttpClient::bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod)
{
	__android_log_print(ANDROID_LOG_DEBUG, "Http",  "VuAndroidHttpClient::bindJavaMethods()\n");

	s_jniEnv = jniEnv;

	// get reference to helper class object
	jstring helperClassName = jniEnv->NewStringUTF("com/vectorunit/VuHttpHelper");
	jclass helperClass = (jclass)jniEnv->CallObjectMethod(classLoaderObject, findClassMethod, helperClassName);
	jniEnv->DeleteLocalRef(helperClassName);

	jmethodID getInstance = jniEnv->GetStaticMethodID(helperClass, "getInstance", "()Lcom/vectorunit/VuHttpHelper;");
	jobject helperObject = jniEnv->CallStaticObjectMethod(helperClass, getInstance);
	s_helperObject = jniEnv->NewGlobalRef(helperObject);

	// methods
	s_openConnection = jniEnv->GetMethodID(helperClass, "openConnection", "(Ljava/lang/String;)Ljava/net/HttpURLConnection;");
	s_setRequestProperty = jniEnv->GetMethodID(helperClass, "setRequestProperty", "(Ljava/net/HttpURLConnection;Ljava/lang/String;Ljava/lang/String;)V");
	s_setTimeoutMS = jniEnv->GetMethodID(helperClass, "setTimeoutMS", "(Ljava/net/HttpURLConnection;I)V");
	s_sendRequest = jniEnv->GetMethodID(helperClass, "sendRequest", "(Ljava/net/HttpURLConnection;Ljava/lang/String;[BI)V");
}

//*****************************************************************************
VUHANDLE VuAndroidHttpClient::createRequest()
{
	return new VuAndroidHttpRequest;
}

//*****************************************************************************
void VuAndroidHttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuAndroidHttpRequest *pRequest = static_cast<VuAndroidHttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuAndroidHttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuAndroidHttpRequest *pRequest = static_cast<VuAndroidHttpRequest *>(hRequest);

	pRequest->mHeader[key] = value;
}

//*****************************************************************************
void VuAndroidHttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuAndroidHttpRequest *pRequest = static_cast<VuAndroidHttpRequest *>(hRequest);

	pRequest->mTimeoutMS = timeoutMS;
}

//*****************************************************************************
void VuAndroidHttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuAndroidHttpRequest *pRequest = static_cast<VuAndroidHttpRequest *>(hRequest);

	pRequest->mStatus = STATUS_WAITING_FOR_RESPONSE;
	pRequest->addRef();

	JNIEnv *jniEnv = s_jniEnv;

    jstring jUrl = jniEnv->NewStringUTF(url);
	jobject jConnection = jniEnv->CallObjectMethod(s_helperObject, s_openConnection, jUrl);
    jniEnv->DeleteLocalRef(jUrl);

	if ( jConnection )
	{
		for ( VuAndroidHttpRequest::Header::const_iterator iter = pRequest->mHeader.begin(); iter != pRequest->mHeader.end(); iter++ )
		{
			jstring jKey = jniEnv->NewStringUTF(iter->first.c_str());
			jstring jValue = jniEnv->NewStringUTF(iter->second.c_str());
			jniEnv->CallVoidMethod(s_helperObject, s_setRequestProperty, jConnection, jKey, jValue);
			jniEnv->DeleteLocalRef(jKey);
			jniEnv->DeleteLocalRef(jValue);
		}

		jniEnv->CallVoidMethod(s_helperObject, s_setTimeoutMS, jConnection, pRequest->mTimeoutMS);

		const char *strMethod = "GET";
		if ( method == METHOD_POST )
			strMethod = "POST";
		jstring jMethod = jniEnv->NewStringUTF(strMethod);

		jbyteArray jData = jniEnv->NewByteArray(size);
		if ( size )
		{
			jbyte *ptr = jniEnv->GetByteArrayElements(jData, NULL);
			memcpy(ptr, data, size);
			jniEnv->ReleaseByteArrayElements(jData, ptr, 0);
		}

		jniEnv->CallVoidMethod(s_helperObject, s_sendRequest, jConnection, jMethod, jData, (int)pRequest);

		jniEnv->DeleteLocalRef(jMethod);
		jniEnv->DeleteLocalRef(jData);
	}
	else
	{
		pRequest->mStatus = STATUS_ERROR;
		pRequest->removeRef();
	}
}

//*****************************************************************************
VuHttpClient::eStatus VuAndroidHttpClient::getStatus(VUHANDLE hRequest)
{
	VuAndroidHttpRequest *pRequest = static_cast<VuAndroidHttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuAndroidHttpClient::getResponse(VUHANDLE hRequest)
{
	VuAndroidHttpRequest *pRequest = static_cast<VuAndroidHttpRequest *>(hRequest);

	return pRequest->mResponse;
}


extern "C"
{
	JNIEXPORT void JNICALL Java_com_vectorunit_VuHttpHelper_onDataReceived(JNIEnv *env, jobject obj, jint id, jbyteArray jData, jint length);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuHttpHelper_onSuccess(JNIEnv *env, jobject obj, jint id);
	JNIEXPORT void JNICALL Java_com_vectorunit_VuHttpHelper_onFailure(JNIEnv *env, jobject obj, jint id);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuHttpHelper_onDataReceived(JNIEnv *env, jobject obj, jint id, jbyteArray jData, jint length)
{
	VuAndroidHttpRequest *pRequest = reinterpret_cast<VuAndroidHttpRequest *>(id);

	int offset = pRequest->mResponse.size();
	pRequest->mResponse.resize(pRequest->mResponse.size() + length);

	jbyte *data = env->GetByteArrayElements(jData, NULL);
	memcpy(&pRequest->mResponse[offset], data, length);
	env->ReleaseByteArrayElements(jData, data, 0);
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuHttpHelper_onSuccess(JNIEnv *env, jobject obj, jint id)
{
	VuAndroidHttpRequest *pRequest = reinterpret_cast<VuAndroidHttpRequest *>(id);

	pRequest->mStatus = VuHttpClient::STATUS_RESPONSE_RECEIVED;
	pRequest->removeRef();
}

//*****************************************************************************
JNIEXPORT void JNICALL Java_com_vectorunit_VuHttpHelper_onFailure(JNIEnv *env, jobject obj, jint id)
{
	VuAndroidHttpRequest *pRequest = reinterpret_cast<VuAndroidHttpRequest *>(id);

	pRequest->mStatus = VuHttpClient::STATUS_ERROR;
	pRequest->removeRef();
}
