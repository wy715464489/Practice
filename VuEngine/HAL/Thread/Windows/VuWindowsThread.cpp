//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows interface class to Thread library.
// 
//*****************************************************************************

#include <thread>
#include "VuEngine/HAL/Thread/VuThread.h"


class VuWindowsThread : public VuThread
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
IMPLEMENT_SYSTEM_COMPONENT(VuThread, VuWindowsThread);


//*****************************************************************************
int VuWindowsThread::getHardwareThreadCount()
{
	return 1;
}

//*****************************************************************************
void VuWindowsThread::setThreadProcessor(int hardwareThread)
{
	// for Windows, don't lock to specific thread
}

//*****************************************************************************
VUHANDLE VuWindowsThread::createThread(void (*pProc)(void *), void *pParam)
{
	return new std::thread(pProc, pParam);
}

//*****************************************************************************
void VuWindowsThread::joinThread(VUHANDLE hThread)
{
	std::thread *pThread = static_cast<std::thread *>(hThread);
	pThread->join();
	delete pThread;
}

//*****************************************************************************
void VuWindowsThread::endThread()
{
}

//*****************************************************************************
VUHANDLE VuWindowsThread::getCurrentThread()
{
	return GetCurrentThread();
}

//*****************************************************************************
VUHANDLE VuWindowsThread::createCriticalSection()
{
	CRITICAL_SECTION *pCS = new CRITICAL_SECTION;
	InitializeCriticalSectionEx(pCS, 0, 0);
	return pCS;
}

//*****************************************************************************
void VuWindowsThread::deleteCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	DeleteCriticalSection(pCS);
	delete pCS;
}

//*****************************************************************************
void VuWindowsThread::enterCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	EnterCriticalSection(pCS);
}

//*****************************************************************************
void VuWindowsThread::leaveCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	LeaveCriticalSection(pCS);
}

//*****************************************************************************
bool VuWindowsThread::tryEnterCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	return TryEnterCriticalSection(pCS) ? true : false;
}

//*****************************************************************************
VUHANDLE VuWindowsThread::createEvent()
{
	return CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);
}

//*****************************************************************************
void VuWindowsThread::destroyEvent(VUHANDLE hEvent)
{
	if ( !CloseHandle(hEvent) )
		VUASSERT(0, "VuWindowsThread::destroyEvent() error!");
}

//*****************************************************************************
void VuWindowsThread::setEvent(VUHANDLE hEvent)
{
	if ( !SetEvent(hEvent) )
		VUASSERT(0, "VuWindowsThread::setEvent() error!");
}

//*****************************************************************************
bool VuWindowsThread::waitForSingleObject(VUHANDLE hEvent, int timeoutMS)
{
	DWORD result = WaitForSingleObjectEx(hEvent, timeoutMS, FALSE);

	if ( result == WAIT_OBJECT_0 )
		return true;

	VUASSERT(result == WAIT_TIMEOUT, "VuWindowsThread::waitForSingleObject() error!");

	return false;
}
