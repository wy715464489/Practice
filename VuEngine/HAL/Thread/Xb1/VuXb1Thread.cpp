//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 interface class to Thread library.
// 
//*****************************************************************************

#include <thread>
#include "VuEngine/HAL/Thread/VuThread.h"


class VuXb1Thread : public VuThread
{
public:
	// cross-platform functionality

	// info
	virtual int			getHardwareThreadCount();
	virtual void		setThreadProcessor(int hardwareThread);

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


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuThread, VuXb1Thread);


//*****************************************************************************
int VuXb1Thread::getHardwareThreadCount()
{
	return 8;
}

//*****************************************************************************
void VuXb1Thread::setThreadProcessor(int hardwareThread)
{
	DWORD mask = 1<<hardwareThread;
	if ( SetThreadAffinityMask(GetCurrentThread(), mask) == 0 )
		VUASSERT(0, "VuXb1Thread::setThreadProcessor() error!");
}

//*****************************************************************************
VUHANDLE VuXb1Thread::createThread(void (*pProc)(void *), void *pParam)
{
	return new std::thread(pProc, pParam);
}

//*****************************************************************************
void VuXb1Thread::joinThread(VUHANDLE hThread)
{
	std::thread *pThread = static_cast<std::thread *>(hThread);
	pThread->join();
	delete pThread;
}

//*****************************************************************************
void VuXb1Thread::endThread()
{
}

//*****************************************************************************
VUHANDLE VuXb1Thread::getCurrentThread()
{
	return GetCurrentThread();
}

//*****************************************************************************
VUHANDLE VuXb1Thread::createCriticalSection()
{
	CRITICAL_SECTION *pCS = new CRITICAL_SECTION;
	InitializeCriticalSectionEx(pCS, 0, 0);
	return pCS;
}

//*****************************************************************************
void VuXb1Thread::deleteCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	DeleteCriticalSection(pCS);
	delete pCS;
}

//*****************************************************************************
void VuXb1Thread::enterCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	EnterCriticalSection(pCS);
}

//*****************************************************************************
void VuXb1Thread::leaveCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	LeaveCriticalSection(pCS);
}

//*****************************************************************************
bool VuXb1Thread::tryEnterCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	return TryEnterCriticalSection(pCS) ? true : false;
}

//*****************************************************************************
VUHANDLE VuXb1Thread::createEvent()
{
	return CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);
}

//*****************************************************************************
void VuXb1Thread::destroyEvent(VUHANDLE hEvent)
{
	if ( !CloseHandle(hEvent) )
		VUASSERT(0, "VuXb1Thread::destroyEvent() error!");
}

//*****************************************************************************
void VuXb1Thread::setEvent(VUHANDLE hEvent)
{
	if ( !SetEvent(hEvent) )
		VUASSERT(0, "VuXb1Thread::setEvent() error!");
}

//*****************************************************************************
bool VuXb1Thread::waitForSingleObject(VUHANDLE hEvent, int timeoutMS)
{
	DWORD result = WaitForSingleObjectEx(hEvent, timeoutMS, FALSE);

	if ( result == WAIT_OBJECT_0 )
		return true;

	VUASSERT(result == WAIT_TIMEOUT, "VuXb1Thread::waitForSingleObject() error!");

	return false;
}
