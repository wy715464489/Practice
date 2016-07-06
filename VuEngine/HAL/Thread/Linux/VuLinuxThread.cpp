//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Linux interface class to Thread library.
//
//*****************************************************************************

#include <pthread.h>
//#include <sys/syscall.h>
#include <errno.h>
#include "VuEngine/HAL/Thread/VuThread.h"


class VuLinuxThread : public VuThread
{
public:
	bool init();

	// cross-platform functionality

	// info
	virtual int			getHardwareThreadCount();
	virtual void		setThreadProcessor(int hardwareThread);

	// create/end
	virtual VUHANDLE	createThread(void (*pProc)(void *), void *pParam);
	virtual void		joinThread(VUHANDLE hThread);
	virtual void		endThread();
	virtual VUHANDLE	getCurrentThread();

	// suspend
	virtual void		sleep(int ms);

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

protected:
	int					mHardwareThreadCount;
};

struct VuLinuxEvent
{
	pthread_mutex_t	mMutex;
	pthread_cond_t	mCondition;
	bool			mbOpen;
	int				mWaitingThreadCount;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuThread, VuLinuxThread);


//*****************************************************************************
bool VuLinuxThread::init()
{
	if ( !VuThread::init() )
		return false;

#if defined VUPS4
	mHardwareThreadCount = 8;
#else
	// determine number of cores
	mHardwareThreadCount = 0;

	if ( FILE *fp = fopen("/proc/cpuinfo", "r") )
	{
		char str[256];
		while(fgets(str, sizeof(str), fp) != NULL)
		{
			if (strncmp(str, "processor", 9) == 0)
				mHardwareThreadCount++;
		}
		fclose(fp);
	}

	if ( mHardwareThreadCount == 0 )
		mHardwareThreadCount = 1;
#endif

	return true;
}

//*****************************************************************************
int VuLinuxThread::getHardwareThreadCount()
{
	return mHardwareThreadCount;
}

//*****************************************************************************
void VuLinuxThread::setThreadProcessor(int hardwareThread)
{
#ifdef VUPS4
	SceKernelCpumask mask = 1<<hardwareThread;
	if ( scePthreadSetaffinity(pthread_self(), mask) != SCE_OK )
		VUASSERT(0, "VuLinuxThread::setThreadProcessor() error!");
#else
	/*
	hardwareThread = hardwareThread%mHardwareThreadCount;

	int mask = 1 << hardwareThread;

	int err, syscallres;
	syscallres = syscall(__NR_sched_setaffinity, 0, sizeof(mask), &mask);
	if (syscallres)
	{
		err = errno;
		VUPRINTF("Error in the syscall setaffinity: mask=%d=0x%x err=%d=0x%x", mask, mask, err, err);
	}
	*/
#endif
}

//*****************************************************************************
VUHANDLE VuLinuxThread::createThread(void (*pProc)(void *), void *pParam)
{
	pthread_t threadId;

	int errVal = pthread_create(&threadId, VUNULL, (void*(*)(void*))pProc, pParam);

	VUASSERT(errVal == 0, "VuLinuxThread::createThread() failed");

	return (VUHANDLE)threadId;
}

//*****************************************************************************
void VuLinuxThread::joinThread(VUHANDLE hThread)
{
	pthread_t threadId = (pthread_t)hThread;

	pthread_join(threadId, VUNULL);
}

//*****************************************************************************
void VuLinuxThread::endThread()
{
	pthread_exit(0);
}

//*****************************************************************************
VUHANDLE VuLinuxThread::getCurrentThread()
{
	return (VUHANDLE)pthread_self();
}

//*****************************************************************************
void VuLinuxThread::sleep(int ms)
{
	timespec args;
	args.tv_sec = ms/1000;
	args.tv_nsec = (ms%1000)*1000000;
	nanosleep(&args, VUNULL);
}

//*****************************************************************************
VUHANDLE VuLinuxThread::createCriticalSection()
{
	pthread_mutex_t *pMutex = new pthread_mutex_t;
	int errVal = pthread_mutex_init(pMutex, VUNULL);
	VUASSERT(errVal == 0, "VuLinuxThread::createCriticalSection() failed");
	return pMutex;
}

//*****************************************************************************
void VuLinuxThread::deleteCriticalSection(VUHANDLE hCriticalSection)
{
	pthread_mutex_t *pMutex = static_cast<pthread_mutex_t *>(hCriticalSection);
	int errVal = pthread_mutex_destroy(pMutex);
	VUASSERT(errVal == 0, "VuLinuxThread::deleteCriticalSection() failed");
	delete pMutex;
}

//*****************************************************************************
void VuLinuxThread::enterCriticalSection(VUHANDLE hCriticalSection)
{
	pthread_mutex_t *pMutex = static_cast<pthread_mutex_t *>(hCriticalSection);
	int errVal = pthread_mutex_lock(pMutex);
	VUASSERT(errVal == 0, "VuLinuxThread::enterCriticalSection() failed");
}

//*****************************************************************************
void VuLinuxThread::leaveCriticalSection(VUHANDLE hCriticalSection)
{
	pthread_mutex_t *pMutex = static_cast<pthread_mutex_t *>(hCriticalSection);
	int errVal = pthread_mutex_unlock(pMutex);
	VUASSERT(errVal == 0, "VuLinuxThread::leaveCriticalSection() failed");
}

//*****************************************************************************
bool VuLinuxThread::tryEnterCriticalSection(VUHANDLE hCriticalSection)
{
	pthread_mutex_t *pMutex = static_cast<pthread_mutex_t *>(hCriticalSection);
	int errVal = pthread_mutex_trylock(pMutex);
	return errVal == 0;
}

//*****************************************************************************
VUHANDLE VuLinuxThread::createEvent()
{
	VuLinuxEvent *pEvent = new VuLinuxEvent;

	pthread_cond_init(&pEvent->mCondition, VUNULL);
	pthread_mutex_init(&pEvent->mMutex, VUNULL);
	pEvent->mbOpen = false;
	pEvent->mWaitingThreadCount = 0;

	return pEvent;
}

//*****************************************************************************
void VuLinuxThread::destroyEvent(VUHANDLE hEvent)
{
	VuLinuxEvent *pEvent = static_cast<VuLinuxEvent *>(hEvent);
	int errVal = pthread_cond_destroy(&pEvent->mCondition);
	VUASSERT(errVal == 0, "VuLinuxThread::destroyEvent() failed");
	errVal = pthread_mutex_destroy(&pEvent->mMutex);
	VUASSERT(errVal == 0, "VuLinuxThread::destroyEvent() failed");
	delete pEvent;
}

//*****************************************************************************
void VuLinuxThread::setEvent(VUHANDLE hEvent)
{
	VuLinuxEvent *pEvent = static_cast<VuLinuxEvent *>(hEvent);

	pthread_mutex_lock(&pEvent->mMutex);
	pEvent->mbOpen = true;
	pthread_mutex_unlock(&pEvent->mMutex);
	pthread_cond_signal(&pEvent->mCondition);
}

//*****************************************************************************
bool VuLinuxThread::waitForSingleObject(VUHANDLE hEvent, int timeoutMS)
{
	VuLinuxEvent *pEvent = static_cast<VuLinuxEvent *>(hEvent);

	pthread_mutex_lock(&pEvent->mMutex);
	pEvent->mWaitingThreadCount++;
	if ( pEvent->mbOpen )
	{
		pEvent->mbOpen = false;
		pEvent->mWaitingThreadCount--;
		pthread_mutex_unlock(&pEvent->mMutex);
		return true;
	}

	int result;
	if ( timeoutMS >= 0 )
	{
		timespec args;
		args.tv_sec = timeoutMS/1000;
		args.tv_nsec = (timeoutMS%1000)*1000000;
		result = pthread_cond_timedwait(&pEvent->mCondition, &pEvent->mMutex, &args);
	}
	else
	{
		result = pthread_cond_wait(&pEvent->mCondition, &pEvent->mMutex);
	}

	if ( result == ETIMEDOUT )
	{
		pEvent->mWaitingThreadCount--;
		pthread_mutex_unlock(&pEvent->mMutex);
		return false;
	}

	pEvent->mbOpen = false;
	pEvent->mWaitingThreadCount--;
	pthread_mutex_unlock(&pEvent->mMutex);
	return true;
}
