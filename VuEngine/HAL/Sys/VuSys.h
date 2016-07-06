//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Sys library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"


class VuEngine;

class VuSys : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuSys)

protected:

	// called by engine
	friend class VuEngine;
	virtual bool	init(const char *forceLanguage, const char *logFileName) { return true; }

public:

	// Error reporting/handling
	virtual bool		error(const char *fmt, ...) = 0;			// show error, raise error condition
	virtual bool		warning(const char *fmt, ...) = 0;			// show warning, then continue without raising error condition
	virtual bool		exitWithError(const char *fmt, ...) = 0;	// show error, then exit
	virtual bool		hasErrors() = 0;							// returns true if errors have been raised
	virtual void		restart(const wchar_t *args) {}				// Instant teardown of app by OS, followed by relaunch from the top 

	// Clock functions
	virtual double		getTime() = 0;
	virtual VUUINT32	getTimeMS() = 0;
	virtual VUUINT64	getPerfCounter() = 0;

	// Debug print
	virtual void		wprintf(const wchar_t *fmt, ...) { printf("WARNING! wprintf() not implemented on this platform!\n"); }
	virtual void		printf(const char *fmt, ...) = 0;
	virtual void		print(const char *str) = 0;

	// Registration of callbacks
	struct LogCallback { virtual void append(const char *str) = 0; };
	virtual void		addLogCallback(LogCallback *pCB) = 0;
	virtual void		removeLogCallback(LogCallback *pCB) = 0;

	// Language/locale
	virtual const char	*getLanguage() = 0;
	virtual const char	*getLocale() = 0;
	virtual const char	*getRegion() = 0;

	// device capabilities
	virtual bool		hasTouch() = 0;
	virtual bool		hasAccel() = 0;
	virtual bool		hasKeyboard() = 0;
	virtual bool		hasMouse() { return false; }

	// user identifier
	virtual const char	*getUserIdentifier() { return ""; }

	// version
	virtual const char	*getVersion() { return "n/a"; }

	// snap view
	virtual bool		needsSnapView() { return false; }
};
