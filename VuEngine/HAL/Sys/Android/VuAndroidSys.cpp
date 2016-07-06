//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Sys library.
// 
//*****************************************************************************

#include <ctype.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "VuAndroidSys.h"
#include "VuEngine/HAL/Thread/VuThread.h"


// defines
#ifndef VURETAIL
	#define USE_NV_PERF 1
#endif
#define MAX_DEBUG_STRING_LENGTH 4096

// nv perfmon
typedef VUUINT64 (* PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC)(void);
typedef VUUINT64 (* PFNEGLGETSYSTEMTIMENVPROC)(void);

PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC eglGetSystemTimeFrequencyNVProc = NULL;
PFNEGLGETSYSTEMTIMENVPROC eglGetSystemTimeNVProc = NULL;

typedef void (*__eglMustCastToProperFunctionPointerType)(void);
__eglMustCastToProperFunctionPointerType GetEglProcAddress(const char *procname)
{
	return eglGetProcAddress(procname);
}


// static JAVA stuff
static JNIEnv		*s_jniEnv;
static jobject		s_helperObject;
static jmethodID	s_handleError;
static jmethodID	s_showToast;
static jmethodID	s_getDeviceId;
static jmethodID	s_getVersion;
static jmethodID	s_hasTouch;


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuAndroidSys);


//*****************************************************************************
void VuAndroidSys::bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod)
{
	__android_log_print(ANDROID_LOG_DEBUG, "sys", "VuAndroidSys::bindJavaMethods()\n");

	s_jniEnv = jniEnv;

	// get reference to helper class object
	jstring helperClassName = jniEnv->NewStringUTF("com/vectorunit/VuSysHelper");
	jclass helperClass = (jclass)jniEnv->CallObjectMethod(classLoaderObject, findClassMethod, helperClassName);
	jniEnv->DeleteLocalRef(helperClassName);

	jmethodID getInstance = jniEnv->GetStaticMethodID(helperClass, "getInstance", "()Lcom/vectorunit/VuSysHelper;");
	jobject helperObject = jniEnv->CallStaticObjectMethod(helperClass, getInstance);
	s_helperObject = jniEnv->NewGlobalRef(helperObject);

	// methods
	s_handleError = jniEnv->GetMethodID(helperClass, "handleError", "(Ljava/lang/String;)V");
	s_showToast = jniEnv->GetMethodID(helperClass, "showToast", "(Ljava/lang/String;)V");
	s_getDeviceId = jniEnv->GetMethodID(helperClass, "getDeviceId", "()Ljava/lang/String;");
	s_getVersion = jniEnv->GetMethodID(helperClass, "getVersion", "()Ljava/lang/String;");
	s_hasTouch = jniEnv->GetMethodID(helperClass, "hasTouch", "()Z");
}

//*****************************************************************************
void VuAndroidSys::handleAndroidError(const char *msg)
{
    jstring jMsg = s_jniEnv->NewStringUTF(msg);
	s_jniEnv->CallVoidMethod(s_helperObject, s_handleError, jMsg);
    s_jniEnv->DeleteLocalRef(jMsg);
}

//*****************************************************************************
void VuAndroidSys::showToast(const char *msg)
{
    jstring jMsg = s_jniEnv->NewStringUTF(msg);
	s_jniEnv->CallVoidMethod(s_helperObject, s_showToast, jMsg);
    s_jniEnv->DeleteLocalRef(jMsg);
}

//*****************************************************************************
VuAndroidSys::VuAndroidSys():
	mbErrorRaised(false),
	mUsingNvPerfMon(false),
	mOsBuildVersion(0),
	mHasTouch(true)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuAndroidSys::~VuAndroidSys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
bool VuAndroidSys::init(const char *forceLanguage, const char *logFileName)
{
	if ( !VuSys::init(forceLanguage, logFileName) )
		return false;

	mPerfFreq = 1000000000; // nanosecond resolution

#if USE_NV_PERF
	bool isTegra = strstr((const char *)glGetString(GL_VENDOR), "NVIDIA") != VUNULL;
	if ( isTegra )
	{
		eglGetSystemTimeFrequencyNVProc = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC)GetEglProcAddress("eglGetSystemTimeFrequencyNV");
		eglGetSystemTimeNVProc = (PFNEGLGETSYSTEMTIMENVPROC)GetEglProcAddress("eglGetSystemTimeNV");

		if ( eglGetSystemTimeFrequencyNVProc && eglGetSystemTimeNVProc )
		{
			mUsingNvPerfMon = true;
			mPerfFreq = eglGetSystemTimeFrequencyNVProc();
		}
	}
#endif

	mPerfInitial = getPerfCounter();

	// get language and country
	mLanguage = forceLanguage;
	if ( mLanguage.empty() )
	{
		std::string language, country;
		{
			jclass localeClass = s_jniEnv->FindClass("java/util/Locale");
			jmethodID localeGetDefault = s_jniEnv->GetStaticMethodID(localeClass, "getDefault", "()Ljava/util/Locale;");
			jobject defaultLocale = s_jniEnv->CallStaticObjectMethod(localeClass, localeGetDefault);

			jmethodID localeGetLanguage = s_jniEnv->GetMethodID(localeClass, "getLanguage", "()Ljava/lang/String;");
			jmethodID localeGetCountry = s_jniEnv->GetMethodID(localeClass, "getCountry", "()Ljava/lang/String;");

			jstring jLanguage = (jstring)s_jniEnv->CallObjectMethod(defaultLocale, localeGetLanguage);
			jstring jCountry = (jstring)s_jniEnv->CallObjectMethod(defaultLocale, localeGetCountry);
			const char *strLanguage = s_jniEnv->GetStringUTFChars(jLanguage, 0);
			const char *strCountry = s_jniEnv->GetStringUTFChars(jCountry, 0);

			language = strLanguage;
			country = strCountry;

			s_jniEnv->ReleaseStringUTFChars(jLanguage, strLanguage);  
			s_jniEnv->ReleaseStringUTFChars(jCountry, strCountry);  
			s_jniEnv->DeleteLocalRef(jLanguage);
			s_jniEnv->DeleteLocalRef(jCountry);

			s_jniEnv->DeleteLocalRef(defaultLocale);
		}

		// determine language
		VUPRINTF("Android Language = %s\n", language.c_str());
		VUPRINTF("Android Country = %s\n", country.c_str());

		mLanguage = "en";

		if ( strncmp(language.c_str(), "de", 2) == 0 )    mLanguage = "de";
		if ( strncmp(language.c_str(), "es", 2) == 0 )    mLanguage = "es";
		if ( strncmp(language.c_str(), "fr", 2) == 0 )    mLanguage = "fr";
		if ( strncmp(language.c_str(), "it", 2) == 0 )    mLanguage = "it";
		if ( strncmp(language.c_str(), "pt", 2) == 0 )    mLanguage = "pt";
		if ( strncmp(language.c_str(), "ja", 2) == 0 )    mLanguage = "ja";
		if ( strncmp(language.c_str(), "ko", 2) == 0 )    mLanguage = "ko";
		if ( strncmp(language.c_str(), "ru", 2) == 0 )    mLanguage = "ru";

		if ( strncmp(language.c_str(), "zh", 2) == 0 )
		{
			if ( strncmp(country.c_str(), "TW", 2) == 0 ) mLanguage = "zh-hant";
			if ( strncmp(country.c_str(), "CN", 2) == 0 ) mLanguage = "zh-hans";
		}
	}

	VUPRINTF("VU Language = %s\n", mLanguage.c_str());

	// get build os version
	{
		jclass versionClass = s_jniEnv->FindClass("android/os/Build$VERSION");
		jfieldID sdkIntFieldID = s_jniEnv->GetStaticFieldID(versionClass, "SDK_INT", "I");
		mOsBuildVersion = s_jniEnv->GetStaticIntField(versionClass, sdkIntFieldID);
	}
	VUPRINTF("os build version = %d\n", mOsBuildVersion);

	// get device id
	{
		jstring jDeviceId = (jstring)s_jniEnv->CallObjectMethod(s_helperObject, s_getDeviceId);
		const char *strDeviceId = s_jniEnv->GetStringUTFChars(jDeviceId, 0);

		mDeviceId = strDeviceId;

		s_jniEnv->ReleaseStringUTFChars(jDeviceId, strDeviceId);  
		s_jniEnv->DeleteLocalRef(jDeviceId);
	}
	VUPRINTF("Device Id = %s\n", mDeviceId.c_str());

	// get version
	{
		jstring jVersion = (jstring)s_jniEnv->CallObjectMethod(s_helperObject, s_getVersion);
		const char *strVersion = s_jniEnv->GetStringUTFChars(jVersion, 0);

		mVersion = strVersion;

		s_jniEnv->ReleaseStringUTFChars(jVersion, strVersion);  
		s_jniEnv->DeleteLocalRef(jVersion);
	}
	VUPRINTF("Version = %s\n", mVersion.c_str());

	// has touch?
	mHasTouch = s_jniEnv->CallBooleanMethod(s_helperObject, s_hasTouch);
	VUPRINTF("Has Touch = %s\n", mHasTouch ? "true" : "false");

	return true;
}

//*****************************************************************************
void VuAndroidSys::release()
{
	if ( hasErrors() )
		exit(-1);
}

//*****************************************************************************
bool VuAndroidSys::error(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	// log the error
	printf("Error: %s\n", str);

	// raise error condition
	mbErrorRaised = true;

	return false;
}

//*****************************************************************************
bool VuAndroidSys::warning(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	// log the warning
	printf("Warning: %s\n", str);

	return false;
}

//*****************************************************************************
bool VuAndroidSys::exitWithError(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	error(str);

	release();

	return false;
}

//*****************************************************************************
bool VuAndroidSys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************
double VuAndroidSys::getTime()
{
	VUUINT64 perfCur = getPerfCounter();
	VUUINT64 perfDelta = perfCur - mPerfInitial;

	double t = (double)perfDelta/(double)mPerfFreq;

	return t;
}

//*****************************************************************************
VUUINT32 VuAndroidSys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuAndroidSys::getPerfCounter()
{
	VUUINT64 val;

	if ( mUsingNvPerfMon )
	{
		val = eglGetSystemTimeNVProc();
	}
	else
	{
		struct timespec res;
		clock_gettime(CLOCK_MONOTONIC, &res);
		val = res.tv_sec*mPerfFreq + res.tv_nsec;
	}

	return val;
}

//*****************************************************************************
void VuAndroidSys::printf(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	print(str);
}

//*****************************************************************************
void VuAndroidSys::print(const char *str)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);

#if !VU_DISABLE_DEBUG_OUTPUT
	__android_log_write(ANDROID_LOG_INFO, "VectorEngine", str);
#endif

	for ( LogCallbacks::iterator iter = mLogCallbacks.begin(); iter != mLogCallbacks.end(); iter++ )
		(*iter)->append(str);

	VuThread::IF()->leaveCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuAndroidSys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuAndroidSys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuAndroidSys::getLanguage()
{
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuAndroidSys::getLocale()
{
	// not implemented yet
	return "UnitedStates";
}

//*****************************************************************************
const char *VuAndroidSys::getRegion()
{
	// not implemented yet
	return "NorthAmericaAll";
}

//*****************************************************************************
int fopen_s(FILE **fp, const char *filename, const char *mode)
{
	*fp = fopen(filename, mode);
	return *fp != 0;
}
