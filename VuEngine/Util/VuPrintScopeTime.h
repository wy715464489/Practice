//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Print Scope Time
// 
//*****************************************************************************

#pragma once


class VuPrintScopeTime
{
public:
	VuPrintScopeTime(const char *fmt, ...);
	~VuPrintScopeTime();

private:
	double	mStartTime;
};

#define VU_PRINT_SCOPE_TIME VuPrintScopeTime scopeTime
