//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Thread library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"


class VuEngine;

class VuThread : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuThread)

protected:

	// called by engine
	friend class VuEngine;
	virtual bool	init() { return true; }

public:

	// info
	virtual int			getHardwareThreadCount() = 0;
	virtual void		setThreadProcessor(int hardwareThread) = 0;

	// create/end
	virtual VUHANDLE	createThread(void (*pProc)(void *), void *pParam) = 0;
	virtual void		joinThread(VUHANDLE hThread) = 0;
	virtual void		endThread() = 0;
	virtual VUHANDLE	getCurrentThread() = 0;

	// suspend
	virtual void		sleep(int ms) {}

	// events
	virtual VUHANDLE	createEvent() = 0;
	virtual void		destroyEvent(VUHANDLE hEvent) = 0;
	virtual void		setEvent(VUHANDLE hEvent) = 0;
	virtual bool		waitForSingleObject(VUHANDLE hEvent, int timeoutMS = -1) = 0;

	// critical sections
	virtual VUHANDLE	createCriticalSection() = 0;
	virtual void		deleteCriticalSection(VUHANDLE hCriticalSection) = 0;
	virtual void		enterCriticalSection(VUHANDLE hCriticalSection) = 0;
	virtual void		leaveCriticalSection(VUHANDLE hCriticalSection) = 0;
	virtual bool		tryEnterCriticalSection(VUHANDLE hCriticalSection) = 0;
};
