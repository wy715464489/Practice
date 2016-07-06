//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class to Thread library.
// 
//*****************************************************************************

#include "VuEngine/HAL/Thread/GenericWin/VuGenericWinThread.h"


class VuWin32Thread : public VuGenericWinThread
{
public:
	// info
	virtual int			getHardwareThreadCount();
	virtual void		setThreadProcessor(int hardwareThread);
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuThread, VuWin32Thread);


//*****************************************************************************
int VuWin32Thread::getHardwareThreadCount()
{
	return 1;
}

//*****************************************************************************
void VuWin32Thread::setThreadProcessor(int hardwareThread)
{
	// for win32, don't lock to specific thread
}
