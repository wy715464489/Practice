//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Print Scope Time
// 
//*****************************************************************************

#include <stdio.h>
#include <stdarg.h>
#include "VuPrintScopeTime.h"


//*****************************************************************************
VuPrintScopeTime::VuPrintScopeTime(const char *fmt, ...)
{
	va_list args;
	char str[256];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	VUPRINTF("%s...", str);
	mStartTime = VuSys::IF()->getTime();
}

//*****************************************************************************
VuPrintScopeTime::~VuPrintScopeTime()
{
	double totalTime = VuSys::IF()->getTime() - mStartTime;

	VUPRINTF(" done (%.2f sec).\n", totalTime);
}
