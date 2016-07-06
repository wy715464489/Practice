//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Screen shot utility
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"


class VuScreenShotWriter
{
public:
	VuScreenShotWriter(int width, int height);
	~VuScreenShotWriter();

	void		write(const VUBYTE *bgr, int size);

private:
	VUHANDLE	mhFile;
};
