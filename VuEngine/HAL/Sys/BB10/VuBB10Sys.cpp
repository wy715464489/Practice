//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  BB10 interface class for Sys.
//
//*****************************************************************************

#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include "VuEngine/HAL/Sys/VuSys.h"
#include "VuEngine/HAL/Thread/VuThread.h"


class VuBB10Sys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();
	
public:
	VuBB10Sys();
	~VuBB10Sys();
	
	// cross-platform functionality
	
	// Error reporting/handling
	virtual bool		error(const char *fmt, ...);			// show error, raise error condition
	virtual bool		warning(const char *fmt, ...);			// show warning, then continue without raising error condition
	virtual bool		exitWithError(const char *fmt, ...);	// show error, then exit
	virtual bool		hasErrors();							// returns true if errors have been raised
	
	// Clock functions
	virtual double		getTime();
	virtual VUUINT32	getTimeMS();
	virtual VUUINT64	getPerfCounter();
	
	// Debug print
	virtual void		printf(const char *fmt, ...);
	virtual void		print(const char *str);
	
	// Registration of callbacks
	virtual void		addLogCallback(LogCallback *pCB);
	virtual void		removeLogCallback(LogCallback *pCB);
	
	// Language/locale
	virtual const char	*getLanguage();
	virtual const char	*getLocale();
	virtual const char	*getRegion();
	
	// device capabilities
	virtual bool		hasTouch() { return true; }
	virtual bool		hasAccel() { return true; }
	virtual bool		hasKeyboard() { return false; }
    
private:
	typedef std::list<VuSys::LogCallback *> LogCallbacks;
	
	VUUINT64			mPerfInitial;
	VUUINT64			mPerfFreq;
	
	bool				mbErrorRaised;
	LogCallbacks		mLogCallbacks;
	
	VUHANDLE			mCriticalSection;
	
	std::string			mLanguage;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuBB10Sys);

// defines
#define MAX_DEBUG_STRING_LENGTH 4096


//*****************************************************************************
VuBB10Sys::VuBB10Sys():
mbErrorRaised(false)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuBB10Sys::~VuBB10Sys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
bool VuBB10Sys::init(const char *forceLanguage, const char *logFileName)
{
	if ( !VuSys::init(forceLanguage, logFileName) )
		return false;
	
	mPerfFreq = 1000000000; // nanosecond resolution
	mPerfInitial = getPerfCounter();
	
	mLanguage = forceLanguage;
	if ( mLanguage.empty() )
		mLanguage = "en";
	
	return true;
}

//*****************************************************************************
void VuBB10Sys::release()
{
	if ( hasErrors() )
		exit(-1);
}

//*****************************************************************************
bool VuBB10Sys::error(const char *fmt, ...)
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
bool VuBB10Sys::warning(const char *fmt, ...)
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
bool VuBB10Sys::exitWithError(const char *fmt, ...)
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
bool VuBB10Sys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************
double VuBB10Sys::getTime()
{
	VUUINT64 perfCur = getPerfCounter();
	VUUINT64 perfDelta = perfCur - mPerfInitial;
	
	double t = (double)perfDelta/(double)mPerfFreq;
	
	return t;
}

//*****************************************************************************
VUUINT32 VuBB10Sys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuBB10Sys::getPerfCounter()
{
	VUUINT64 val;

	struct timespec res;
	clock_gettime(CLOCK_MONOTONIC, &res);
	val = res.tv_sec*mPerfFreq + res.tv_nsec;

	return val;
}

//*****************************************************************************
void VuBB10Sys::getLocalTime(VuSystemTime &systemTime)
{
	time_t timeVal;
	timeVal = time(&timeVal);
	tm sysTime;
	localtime_r(&timeVal, &sysTime);
	
	systemTime.mYear = sysTime.tm_year;
	systemTime.mMonth = sysTime.tm_mon + 1;
	systemTime.mDay = sysTime.tm_mday;
	systemTime.mHour = sysTime.tm_hour;
	systemTime.mMinute = sysTime.tm_min;
	systemTime.mSecond = sysTime.tm_sec;
}

//*****************************************************************************
void VuBB10Sys::printf(const char *fmt, ...)
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
void VuBB10Sys::print(const char *str)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);
	
	for ( LogCallbacks::iterator iter = mLogCallbacks.begin(); iter != mLogCallbacks.end(); iter++ )
		(*iter)->append(str);
	
	VuThread::IF()->leaveCriticalSection(mCriticalSection);
	
	::printf("%s", str);
	fflush(stdout);
}

//*****************************************************************************
void VuBB10Sys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuBB10Sys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuBB10Sys::getLanguage()
{
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuBB10Sys::getLocale()
{
	// not implemented yet
	return "UnitedStates";
}

//*****************************************************************************
const char *VuBB10Sys::getRegion()
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
