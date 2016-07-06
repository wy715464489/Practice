//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows interface class for Sys.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Sys/VuSys.h"


class VuWindowsSys : public VuSys
{
protected:
	virtual bool	init(const char *forceLanguage, const char *logFileName);
	virtual void	release();

public:
	VuWindowsSys();
	~VuWindowsSys();

	// platform-specific functionality
	static void			setLanguage(const std::string &language);
	static void			setHasTouch(bool hasTouch) { smHasTouch = hasTouch; }
	static void			setHasAccel(bool hasAccel) { smHasAccel = hasAccel; }
	static void			setHasKeyboard(bool hasKeyboard) { smHasKeyboard = hasKeyboard; }
	static void			setHasMouse(bool hasMouse) { smHasMouse = hasMouse; }

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
	virtual bool		hasTouch() { return smHasTouch; }
	virtual bool		hasAccel() { return smHasAccel; }
	virtual bool		hasKeyboard() { return smHasKeyboard; }

	// snap view
	virtual bool		needsSnapView() { return true; }

	// platform-specific functionality
	static VuWindowsSys *IF() { return static_cast<VuWindowsSys *>(VuSys::IF()); }

	static void											setSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel ^swapChainPanel) { smSwapChainPanel = swapChainPanel; }
	static Windows::UI::Xaml::Controls::SwapChainPanel	^getSwapChainPanel() { return smSwapChainPanel; }

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
	static bool			smHasTouch;
	static bool			smHasAccel;
	static bool			smHasKeyboard;
	static bool			smHasMouse;

	static Windows::UI::Xaml::Controls::SwapChainPanel ^smSwapChainPanel;
};