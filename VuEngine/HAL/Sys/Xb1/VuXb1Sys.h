//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 interface class for Sys.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Sys/VuSys.h"

/*
Product Id     fbc66a30-a43e-40e1-b1fd-34daa07c6f69
SCID           76150100-d0df-4851-b679-1c8f304863f0
Title ID       304863F0  
Sandbox Id     VCTR.0
Sandbox Id     ad99d363-8da4-499f-9989-59cecce20e83
*/

#define VU_SCID			"76150100-d0df-4851-b679-1c8f304863f0"
#define VU_TITLE_ID		"304863F0"
#define VU_TITLE_ID_HEX	0x304863F0
#define VU_SANDBOX		"VCTR.0"


class VuXb1Sys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();

public:
	VuXb1Sys();
	~VuXb1Sys();

	// platform-specific functionality
	static void			setLanguage(const std::string &language);
	static void			setLocale(const std::string &locale);

	static void			setMainWindowActivated(bool activated);
	static bool			getMainWindowActivated(); 

	// cross-platform functionality

	// Error reporting/handling
	virtual bool		error(const char *fmt, ...);			// show error, raise error condition
	virtual bool		warning(const char *fmt, ...);			// show warning, then continue without raising error condition
	virtual bool		exitWithError(const char *fmt, ...);	// show error, then exit
	virtual bool		hasErrors();							// returns true if errors have been raised
	virtual void		restart(const wchar_t *args);		// Instant teardown of app by OS, followed by relaunch from the top 

	// Clock functions
	virtual double		getTime();
	virtual VUUINT32	getTimeMS();
	virtual VUUINT64	getPerfCounter();

	// Debug print
	virtual void		wprintf(const wchar_t *fmt, ...);
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
	virtual bool		hasTouch() { return false; }
	virtual bool		hasAccel() { return false; }
	virtual bool		hasKeyboard() { return false; }

private:
	typedef std::list<VuSys::LogCallback *> LogCallbacks;

	__int64			mPerfFreq;
	__int64			mPerfInitial;

	bool			mbErrorRaised;
	LogCallbacks	mLogCallbacks;

	VUHANDLE		mCriticalSection;

	std::string		mLanguage;
	std::string		mLogFileName;

	static std::string	smLanguage;
	static std::string	smLocale;

	static bool		smMainWindowActivated;
};