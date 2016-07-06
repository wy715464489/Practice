//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  iOS interface class for Sys.
//
//*****************************************************************************

#import <AdSupport/ASIdentifierManager.h>
#include <mach/mach_time.h>
#include "VuEngine/HAL/Sys/VuSys.h"
#include "VuEngine/HAL/Thread/VuThread.h"


class VuIosSys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();
	
public:
	VuIosSys();
	~VuIosSys();
	
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
    
	// user identifier
	virtual const char	*getUserIdentifier() { return mUserIdentifier.c_str(); }
	
    // version
	virtual const char	*getVersion() { return mVersionNumber.c_str(); }

private:
	typedef std::list<VuSys::LogCallback *> LogCallbacks;
	
	VUUINT64			mPerfInitial;
	VUUINT64			mPerfFreq;
	
	bool				mbErrorRaised;
	LogCallbacks		mLogCallbacks;
	
	VUHANDLE			mCriticalSection;
	
	std::string			mLanguage;
	std::string			mUserIdentifier;
    std::string         mVersionNumber;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuIosSys);

// defines
#define MAX_DEBUG_STRING_LENGTH 4096


//*****************************************************************************
VuIosSys::VuIosSys():
mbErrorRaised(false)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuIosSys::~VuIosSys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
bool VuIosSys::init(const char *forceLanguage, const char *logFileName)
{
	if ( !VuSys::init(forceLanguage, logFileName) )
		return false;
	
	mach_timebase_info_data_t timeInfo;
	mach_timebase_info(&timeInfo);
	
	mPerfFreq = 1000000000;
	mPerfFreq *= timeInfo.denom;
	mPerfFreq /= timeInfo.numer;
	
	mPerfInitial = getPerfCounter();
	
	mLanguage = forceLanguage;
	if ( mLanguage.empty() )
		mLanguage = "en";
	
	// user identifier
	if ( NSClassFromString(@"ASIdentifierManager") )
	{
		NSString *ifa = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
		mUserIdentifier = [ifa UTF8String];
	}
    
    mVersionNumber = [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] UTF8String];
    
	return true;
}

//*****************************************************************************
void VuIosSys::release()
{
	if ( hasErrors() )
		exit(-1);
}

//*****************************************************************************
bool VuIosSys::error(const char *fmt, ...)
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
bool VuIosSys::warning(const char *fmt, ...)
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
bool VuIosSys::exitWithError(const char *fmt, ...)
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
bool VuIosSys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************
double VuIosSys::getTime()
{
	VUUINT64 perfCur = getPerfCounter();
	VUUINT64 perfDelta = perfCur - mPerfInitial;
	
	double t = (double)perfDelta/(double)mPerfFreq;
	
	return t;
}

//*****************************************************************************
VUUINT32 VuIosSys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuIosSys::getPerfCounter()
{
	return mach_absolute_time();
}

//*****************************************************************************
void VuIosSys::printf(const char *fmt, ...)
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
void VuIosSys::print(const char *str)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);
	
	for ( LogCallbacks::iterator iter = mLogCallbacks.begin(); iter != mLogCallbacks.end(); iter++ )
		(*iter)->append(str);
	
	VuThread::IF()->leaveCriticalSection(mCriticalSection);
	
	::printf("%s", str);
}

//*****************************************************************************
void VuIosSys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuIosSys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuIosSys::getLanguage()
{
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuIosSys::getLocale()
{
	// not implemented yet
	return "UnitedStates";
}

//*****************************************************************************
const char *VuIosSys::getRegion()
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

