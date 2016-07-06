//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GenericWin interface class to Thread library.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Thread/VuThread.h"


class VuGenericWinThread : public VuThread
{
public:
	// cross-platform functionality

	// create/end
	virtual VUHANDLE	createThread(void (*pProc)(void *), void *pParam);
	virtual void		joinThread(VUHANDLE hThread);
	virtual void		endThread();
	virtual VUHANDLE	getCurrentThread();

	// events
	virtual VUHANDLE	createEvent();
	virtual void		destroyEvent(VUHANDLE hEvent);
	virtual void		setEvent(VUHANDLE hEvent);
	virtual bool		waitForSingleObject(VUHANDLE hEvent, int timeoutMS);

	// critical sections
	virtual VUHANDLE	createCriticalSection();
	virtual void		deleteCriticalSection(VUHANDLE hCriticalSection);
	virtual void		enterCriticalSection(VUHANDLE hCriticalSection);
	virtual void		leaveCriticalSection(VUHANDLE hCriticalSection);
	virtual bool		tryEnterCriticalSection(VUHANDLE hCriticalSection);
};