//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Sys library.
// 
//*****************************************************************************

#include <process.h>
#include "VuWindowsSys.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Util/VuUtf8.h"


// defines

#define MAX_DEBUG_STRING_LENGTH 4096


// static variables
std::string VuWindowsSys::smLanguage = "en";
bool VuWindowsSys::smHasTouch = false;
bool VuWindowsSys::smHasAccel = false;
bool VuWindowsSys::smHasKeyboard = false;
bool VuWindowsSys::smHasMouse = false;

Windows::UI::Xaml::Controls::SwapChainPanel ^VuWindowsSys::smSwapChainPanel;


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuWindowsSys);


//*****************************************************************************
VuWindowsSys::VuWindowsSys():
	mbErrorRaised(false)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuWindowsSys::~VuWindowsSys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuWindowsSys::setLanguage(const std::string &language)
{
	smLanguage = "en";

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
}

//*****************************************************************************
bool VuWindowsSys::init(const char *forceLanguage, const char *logFileName)
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
			mLanguage[i] = tolower(mLanguage[i]);
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
void VuWindowsSys::release()
{
	if ( hasErrors() )
		exit(-1);
}

//*****************************************************************************
bool VuWindowsSys::error(const char *fmt, ...)
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
bool VuWindowsSys::warning(const char *fmt, ...)
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
bool VuWindowsSys::exitWithError(const char *fmt, ...)
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
bool VuWindowsSys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************
double VuWindowsSys::getTime()
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
VUUINT32 VuWindowsSys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuWindowsSys::getPerfCounter()
{
	LARGE_INTEGER li;
	if ( QueryPerformanceCounter(&li) )
		return li.QuadPart;

	return 0;
}

//*****************************************************************************
void VuWindowsSys::printf(const char *fmt, ...)
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
void VuWindowsSys::print(const char *str)
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
void VuWindowsSys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuWindowsSys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuWindowsSys::getLanguage()
{
	// not implemented yet
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuWindowsSys::getLocale()
{
	// not implemented yet
	return "UnitedStates";
}

//*****************************************************************************
const char *VuWindowsSys::getRegion()
{
	// not implemented yet
	return "NorthAmericaAll";
}
