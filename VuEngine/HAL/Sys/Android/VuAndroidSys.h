//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android interface class for Sys.
//
//*****************************************************************************

#pragma once

#include <jni.h>
#include "VuEngine/HAL/Sys/VuSys.h"


class VuAndroidSys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();

public:
	// this function MUST be called from the application's JNI_OnLoad,
	// from a function known to be called by JNI_OnLoad, or from a function
	// in a Java-called thread.
	static void			bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod);

	static void			handleAndroidError(const char *msg);
	static void			showToast(const char *msg);

	VuAndroidSys();
	~VuAndroidSys();

	// platform-specific functionality
	static VuAndroidSys *IF() { return static_cast<VuAndroidSys *>(VuSys::IF()); }

	int		getOsBuildVersion() { return mOsBuildVersion; }

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
	virtual bool		hasTouch() { return mHasTouch; }
	virtual bool		hasAccel() { return true; }
	virtual bool		hasKeyboard() { return false; }

	// user identifier
	virtual const char	*getUserIdentifier() { return mDeviceId.c_str(); }

	// version
	virtual const char	*getVersion() { return mVersion.c_str(); }

private:
	typedef std::list<VuSys::LogCallback *> LogCallbacks;

	VUUINT64		mPerfInitial;
	VUUINT64		mPerfFreq;

	bool			mbErrorRaised;
	LogCallbacks	mLogCallbacks;

	VUHANDLE		mCriticalSection;
	bool			mUsingNvPerfMon;

	std::string		mLanguage;
	int				mOsBuildVersion;
	std::string		mDeviceId;
	std::string		mVersion;
	bool			mHasTouch;
};