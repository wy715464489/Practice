//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class for Net.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Net/VuNet.h"


class VuWin32Net : public VuNet
{
protected:
	virtual bool init();
	virtual void release();
};
