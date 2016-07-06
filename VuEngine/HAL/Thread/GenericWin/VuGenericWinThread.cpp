//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GenericWin interface class to Thread library.
// 
//*****************************************************************************

#include "VuGenericWinThread.h"


typedef void (Procedure)(void *);

struct ParamWrapper
{
	Procedure	*mpProc;
	void		*mpParam;
};

DWORD WINAPI ThreadWrapper(void *params)
{
	ParamWrapper *pParamWrapper = static_cast<ParamWrapper *>(params);

	Procedure *pProc = pParamWrapper->mpProc;
	void *pParam = pParamWrapper->mpParam;
	delete pParamWrapper;

	(*pProc)(pParam);

	return 0;
}

//*****************************************************************************
VUHANDLE VuGenericWinThread::createThread(void (*pProc)(void *), void *pParam)
{
	ParamWrapper *pParamWrapper = new ParamWrapper;
	pParamWrapper->mpProc = pProc;
	pParamWrapper->mpParam = pParam;

	VUHANDLE hThread = CreateThread(NULL, 0, ThreadWrapper, pParamWrapper, 0, NULL);

	VUASSERT(hThread, "VuGenericWinThread::createThread() failed to create thread");

	return (VUHANDLE)hThread;
}

//*****************************************************************************
void VuGenericWinThread::joinThread(VUHANDLE hThread)
{
	WaitForSingleObjectEx(hThread, INFINITE, FALSE);
}

//*****************************************************************************
void VuGenericWinThread::endThread()
{
}

//*****************************************************************************
VUHANDLE VuGenericWinThread::getCurrentThread()
{
	return GetCurrentThread();
}

//*****************************************************************************
VUHANDLE VuGenericWinThread::createCriticalSection()
{
	CRITICAL_SECTION *pCS = new CRITICAL_SECTION;
	InitializeCriticalSectionEx(pCS, 0, 0);
	return pCS;
}

//*****************************************************************************
void VuGenericWinThread::deleteCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	DeleteCriticalSection(pCS);
	delete pCS;
}

//*****************************************************************************
void VuGenericWinThread::enterCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	EnterCriticalSection(pCS);
}

//*****************************************************************************
void VuGenericWinThread::leaveCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	LeaveCriticalSection(pCS);
}

//*****************************************************************************
bool VuGenericWinThread::tryEnterCriticalSection(VUHANDLE hCriticalSection)
{
	CRITICAL_SECTION *pCS = static_cast<CRITICAL_SECTION *>(hCriticalSection);
	return TryEnterCriticalSection(pCS) ? true : false;
}

//*****************************************************************************
VUHANDLE VuGenericWinThread::createEvent()
{
	return CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);
}

//*****************************************************************************
void VuGenericWinThread::destroyEvent(VUHANDLE hEvent)
{
	if ( !CloseHandle(hEvent) )
		VUASSERT(0, "VuGenericWinThread::destroyEvent() error!");
}

//*****************************************************************************
void VuGenericWinThread::setEvent(VUHANDLE hEvent)
{
	if ( !SetEvent(hEvent) )
		VUASSERT(0, "VuGenericWinThread::setEvent() error!");
}

//*****************************************************************************
bool VuGenericWinThread::waitForSingleObject(VUHANDLE hEvent, int timeoutMS)
{
	DWORD result = WaitForSingleObjectEx(hEvent, timeoutMS, FALSE);

	if ( result == WAIT_OBJECT_0 )
		return true;

	VUASSERT(result == WAIT_TIMEOUT, "VuGenericWinThread::waitForSingleObject() error!");

	return false;
}
