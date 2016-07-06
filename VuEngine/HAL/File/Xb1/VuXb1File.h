//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 interface class for File.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/File/GenericWin/VuGenericWinFile.h"


class VuXb1File : public VuGenericWinFile
{
public:
	virtual bool	init(const std::string &rootPath, const std::string &projectName);
};
