//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Sys library.
// 
//*****************************************************************************

#include <process.h>
#include "VuXb1Sys.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Util/VuUtf8.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Xbox::ApplicationModel;

// defines

#define MAX_DEBUG_STRING_LENGTH 4096


// static variables
std::string VuXb1Sys::smLanguage = "en";
std::string VuXb1Sys::smLocale = "US";
bool VuXb1Sys::smMainWindowActivated = false;

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuXb1Sys);


//*****************************************************************************
VuXb1Sys::VuXb1Sys():
	mbErrorRaised(false)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuXb1Sys::~VuXb1Sys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuXb1Sys::setLanguage(const std::string &language)
{
	if ( strncmp(language.c_str(), "en", 2) == 0 )      smLanguage = "en";
	if ( strncmp(language.c_str(), "de", 2) == 0 )      smLanguage = "de";
	if ( strncmp(language.c_str(), "es", 2) == 0 )      smLanguage = "es";
	if ( strncmp(language.c_str(), "fr", 2) == 0 )      smLanguage = "fr";
	if ( strncmp(language.c_str(), "it", 2) == 0 )      smLanguage = "it";
	if ( strncmp(language.c_str(), "pt", 2) == 0 )      smLanguage = "pt";
	if ( strncmp(language.c_str(), "ja", 2) == 0 )      smLanguage = "ja";
	if ( strncmp(language.c_str(), "ko", 2) == 0 )      smLanguage = "ko";
	if ( strncmp(language.c_str(), "ru", 2) == 0 )      smLanguage = "ru";
	if ( strncmp(language.c_str(), "zh-Hant", 7) == 0 ) smLanguage = "zh-hant";
	if ( strncmp(language.c_str(), "zh-Hans", 7) == 0 ) smLanguage = "zh-hans";
	if ( strncmp(language.c_str(), "zh-HK", 5) == 0 ) smLanguage = "zh-hant";
	if ( strncmp(language.c_str(), "zh-TW", 5) == 0 ) smLanguage = "zh-hant";
}

//*****************************************************************************
void VuXb1Sys::setLocale(const std::string &locale)
{
	smLocale = locale;
}

//*****************************************************************************
bool VuXb1Sys::init(const char *forceLanguage, const char *logFileName)
{
	if ( !VuSys::init(forceLanguage, logFileName) )
		return false;

	LARGE_INTEGER li;

	if ( !QueryPerformanceFrequency(&li) )
		return false;
	mPerfFreq = li.QuadPart;

	if ( !QueryPerformanceCounter(&li) )
		return false;
	mPerfInitial = li.QuadPart;

	mLanguage = forceLanguage;
	if ( mLanguage.empty() )
	{
		mLanguage = smLanguage;
		for ( int i = 0; i < (int)mLanguage.size(); i++ )
			mLanguage[i] = (char)tolower(mLanguage[i]);
	}

	mLogFileName = logFileName;

	// clear log file
	if ( !mLogFileName.empty() )
	{
		FILE *fp;
		fopen_s(&fp, mLogFileName.c_str(), "w");
		fclose(fp);
	}

	return true;
}

//*****************************************************************************
void VuXb1Sys::release()
{
	if ( hasErrors() )
		exit(-1);
}

//*****************************************************************************
bool VuXb1Sys::error(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	// log the error
	printf("Error: %s\n", str);

	//// show the error
	//showMessageBox("Error", str);

	// raise error condition
	mbErrorRaised = true;

	return false;
}

//*****************************************************************************
bool VuXb1Sys::warning(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	// log the warning
	printf("Warning: %s\n", str);

	//// show the error
	//showMessageBox("Warning", str);

	return false;
}

//*****************************************************************************
bool VuXb1Sys::exitWithError(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	error(str);

	release();

	return false;
}

//*****************************************************************************
bool VuXb1Sys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************

void VuXb1Sys::restart(const wchar_t *args)
{
	Platform::String^ commandLineArgs = ref new Platform::String(args);

	CoreApplication::RestartApplicationOnly(commandLineArgs, nullptr);

	return;
}

//*****************************************************************************
double VuXb1Sys::getTime()
{
	double t = 0.0;

	LARGE_INTEGER li;
	if ( QueryPerformanceCounter(&li) )
	{
		__int64 iPerfCur = li.QuadPart;
		__int64 iPerfDelta = iPerfCur - mPerfInitial;
		t = (double)iPerfDelta/(double)mPerfFreq;
	}

	return t;
}

//*****************************************************************************
VUUINT32 VuXb1Sys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuXb1Sys::getPerfCounter()
{
	LARGE_INTEGER li;
	if ( QueryPerformanceCounter(&li) )
		return li.QuadPart;

	return 0;
}

//*****************************************************************************
void VuXb1Sys::wprintf(const wchar_t *fmt, ...)
{
	va_list args;

	wchar_t str[MAX_DEBUG_STRING_LENGTH];
	memset(str, 0, sizeof(str));

	va_start(args, fmt);
	_vsnwprintf_s(str, sizeof(str), fmt, args);
	va_end(args);

	std::wstring wideString(str);
	std::string narrowString(wideString.begin(), wideString.end());

	print(narrowString.c_str());
}

//*****************************************************************************
void VuXb1Sys::printf(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str), fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	print(str);
}

//*****************************************************************************
void VuXb1Sys::print(const char *str)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);

#if !VU_DISABLE_DEBUG_OUTPUT
	wchar_t wstr[MAX_DEBUG_STRING_LENGTH];
	VuUtf8::convertUtf8StringToWCharString(str, wstr, MAX_DEBUG_STRING_LENGTH);
	OutputDebugStringW(wstr);
#endif

	for ( LogCallbacks::iterator iter = mLogCallbacks.begin(); iter != mLogCallbacks.end(); iter++ )
		(*iter)->append(str);

	// append to log file
	if ( !mLogFileName.empty() )
	{
		FILE *fp;
		fopen_s(&fp, mLogFileName.c_str(), "a");
		fputs(str, fp);
		fclose(fp);
	}

	VuThread::IF()->leaveCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuXb1Sys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuXb1Sys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuXb1Sys::getLanguage()
{
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuXb1Sys::getLocale()
{
	return smLocale.c_str();
}

//*****************************************************************************
const char *VuXb1Sys::getRegion()
{
	if (smLocale == "US" || smLocale == "CA" || smLocale == "MX")
	{
		return "NorthAmerica";
	}

	return "ForeignLands";
}

//*****************************************************************************
void VuXb1Sys::setMainWindowActivated(bool activated)
{
	smMainWindowActivated = activated;
}

//*****************************************************************************
bool VuXb1Sys::getMainWindowActivated()
{
	return smMainWindowActivated;
}

