//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Configures settings for release configuration.
// 
//*****************************************************************************

#pragma once


#define VUPRINTF(x, ...)	VuSys::IF()->printf(x, ##__VA_ARGS__)
#define VUWPRINTF(x, ...)	VuSys::IF()->wprintf(x, ##__VA_ARGS__)
#define VUPRINT(x)			VuSys::IF()->print(x)
#define VUERROR(x, ...)		VuSys::IF()->exitWithError(x, ##__VA_ARGS__)
#define VUWARNING(x, ...)	VuSys::IF()->warning(x, ##__VA_ARGS__)
