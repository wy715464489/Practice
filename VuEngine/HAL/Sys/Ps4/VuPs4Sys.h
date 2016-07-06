//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 Interface class to Sys library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Sys/VuSys.h"

// defines
#define MAX_DEBUG_STRING_LENGTH 4096

enum eSonyBuildId
{
	BUILD_ID_SCEA,
	BUILD_ID_SCEE,
	BUILD_ID_SCEJ
};

class VuPs4Sys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();
	
public:
	VuPs4Sys();
	~VuPs4Sys();

	// platform-specific functionality
	static void			setLanguage(const std::string &language);
	static void			setLocale(const std::string &locale);

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
	virtual bool		hasTouch() { return false; }
	virtual bool		hasAccel() { return false; }
	virtual bool		hasKeyboard() { return false; }
    
	// user identifier
	virtual const char	*getUserIdentifier() { return mUserIdentifier.c_str(); }
	
	// platform-specific functionality
	static VuPs4Sys *IF() { return static_cast<VuPs4Sys *>(VuSys::IF()); }

	eSonyBuildId		getSonyBuildId() { return mSonyBuildId; }

	// Accessors for global PS4 ID's
	static int			getNetPoolID();
	static int			getSslContextID();
	static int			getHttpContextID();

	// Are we allowed to transmit over the network in any fashion?
	static bool			getNetworkRestricted() { return smNetworkRestricted; }
	static void			setNetworkRestricted(bool isRestricted) { smNetworkRestricted = isRestricted; }

private:
	typedef std::list<VuSys::LogCallback *> LogCallbacks;
	
	VUUINT64			mPerfInitial;
	VUUINT64			mPerfFreq;
	
	bool				mbErrorRaised;
	LogCallbacks		mLogCallbacks;
	
	VUHANDLE			mCriticalSection;
	
	std::string			mLanguage;
	std::string			mUserIdentifier;

	eSonyBuildId		mSonyBuildId;

	static std::string	smLanguage;
	static std::string	smLocale;
	static bool			smNetworkRestricted;
};

