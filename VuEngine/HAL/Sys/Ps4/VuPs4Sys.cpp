//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 interface class for Sys.
//
//*****************************************************************************
#include <libdbg.h>
#include <app_content.h>

#include "VuEngine/HAL/Thread/VuThread.h"

#include "VuEngine/HAL/Sys/Ps4/VuPs4Sys.h"

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuPs4Sys);

// static variables
std::string VuPs4Sys::smLanguage = "en";
std::string VuPs4Sys::smLocale = "US";

bool VuPs4Sys::smNetworkRestricted = true;

//*****************************************************************************
VuPs4Sys::VuPs4Sys():
mbErrorRaised(false)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuPs4Sys::~VuPs4Sys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
bool VuPs4Sys::init(const char *forceLanguage, const char *logFileName)
{
	if ( !VuSys::init(forceLanguage, logFileName) )
		return false;
	
	mPerfFreq = 1000000000; // nanosecond resolution

	mPerfInitial = getPerfCounter();
	
	mLanguage = forceLanguage;
	if (mLanguage.empty())
	{
		mLanguage = smLanguage;
		for (int i = 0; i < (int)mLanguage.size(); i++)
			mLanguage[i] = (char)tolower(mLanguage[i]);
	}

	VUINT value = -1;

	VUINT result = sceAppContentAppParamGetInt(SCE_APP_CONTENT_APPPARAM_ID_USER_DEFINED_PARAM_1, &value);
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: Unable to get User Definied Param 1. Error=%0.8x This is used to determine which Sony build this is. Defaulting to SCEA\n", result);

		result = 0;
	}

	mSonyBuildId = static_cast<eSonyBuildId>(value);

	return true;
}

//*****************************************************************************
void VuPs4Sys::release()
{
	if ( hasErrors() )
	{
		SCE_BREAK();
		exit(-1);
	}
}

//*****************************************************************************
bool VuPs4Sys::error(const char *fmt, ...)
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
bool VuPs4Sys::warning(const char *fmt, ...)
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
bool VuPs4Sys::exitWithError(const char *fmt, ...)
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
bool VuPs4Sys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************
double VuPs4Sys::getTime()
{
	VUUINT64 perfCur = getPerfCounter();
	VUUINT64 perfDelta = perfCur - mPerfInitial;
	
	double t = (double)perfDelta/(double)mPerfFreq;
	
	return t;
}

//*****************************************************************************
VUUINT32 VuPs4Sys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuPs4Sys::getPerfCounter()
{
	SceKernelTimespec ts;
	sceKernelClockGettime(SCE_KERNEL_CLOCK_MONOTONIC, &ts);
	return ts.tv_sec*mPerfFreq + ts.tv_nsec;
}

//*****************************************************************************
void VuPs4Sys::printf(const char *fmt, ...)
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
void VuPs4Sys::print(const char *str)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);
	
	for ( LogCallbacks::iterator iter = mLogCallbacks.begin(); iter != mLogCallbacks.end(); iter++ )
		(*iter)->append(str);
	
	VuThread::IF()->leaveCriticalSection(mCriticalSection);
	
	::printf("%s", str);
}

//*****************************************************************************
void VuPs4Sys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuPs4Sys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuPs4Sys::getLanguage()
{
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuPs4Sys::getLocale()
{
	return smLocale.c_str();
}

//*****************************************************************************
const char *VuPs4Sys::getRegion()
{
	if (smLocale == "US" || smLocale == "CA" || smLocale == "MX")
	{
		return "NorthAmerica";
	}

	return "ForeignLands";
}

//*****************************************************************************
void VuPs4Sys::setLanguage(const std::string &language)
{
	if (strncmp(language.c_str(), "en", 2) == 0)      smLanguage = "en";
	if (strncmp(language.c_str(), "de", 2) == 0)      smLanguage = "de";
	if (strncmp(language.c_str(), "es", 2) == 0)      smLanguage = "es";
	if (strncmp(language.c_str(), "fr", 2) == 0)      smLanguage = "fr";
	if (strncmp(language.c_str(), "it", 2) == 0)      smLanguage = "it";
	if (strncmp(language.c_str(), "pt", 2) == 0)      smLanguage = "pt";
	if (strncmp(language.c_str(), "ja", 2) == 0)      smLanguage = "ja";
	if (strncmp(language.c_str(), "ko", 2) == 0)      smLanguage = "ko";
	if (strncmp(language.c_str(), "ru", 2) == 0)      smLanguage = "ru";
	if (strncmp(language.c_str(), "zh-Hant", 7) == 0) smLanguage = "zh-hant";
	if (strncmp(language.c_str(), "zh-Hans", 7) == 0) smLanguage = "zh-hans";
	if (strncmp(language.c_str(), "zh-HK", 5) == 0) smLanguage = "zh-hant";
	if (strncmp(language.c_str(), "zh-TW", 5) == 0) smLanguage = "zh-hant";
}

//*****************************************************************************
void VuPs4Sys::setLocale(const std::string &locale)
{
	smLocale = locale;
}

// PS4 Manager Handles/Contexts/ID's
int VuPs4Sys::getNetPoolID() { extern int sNetPoolID; return sNetPoolID; }
int VuPs4Sys::getSslContextID() { extern int sSslContextID;  return sSslContextID; }
int VuPs4Sys::getHttpContextID() { extern int sHttpContextID;  return sHttpContextID; }

