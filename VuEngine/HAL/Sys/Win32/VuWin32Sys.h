//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class for Sys.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Sys/VuSys.h"


class VuWin32Sys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();

public:
	VuWin32Sys();
	~VuWin32Sys();

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
	virtual bool		hasTouch();
	virtual bool		hasAccel() { return false; }
	virtual bool		hasKeyboard() { return true; }
	virtual bool		hasMouse() { return true; }

	// user identifier
	virtual const char	*getUserIdentifier();

	// snap view
	virtual bool		needsSnapView() { return true; }

	// platform-specific functionality
	static VuWin32Sys *IF() { return static_cast<VuWin32Sys *>(VuSys::IF()); }

	bool			createProcess(const char *strApplicationName, const char *strCommandLine, const char *strCurDir, bool bWait, bool bLog = true);

private:
	typedef std::list<VuSys::LogCallback *> LogCallbacks;

	void			printOutput(HANDLE hStdOut_Read);
	void			showMessageBox(const char *strTitle, const char *strText);

	__int64			mPerfFreq;
	__int64			mPerfInitial;

	bool			mbErrorRaised;
	LogCallbacks	mLogCallbacks;

	VUHANDLE		mCriticalSection;

	std::string		mLanguage;
	std::string		mUserIdentifier;
};