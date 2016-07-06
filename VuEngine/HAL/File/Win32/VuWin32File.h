//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class for File.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/File/GenericWin/VuGenericWinFile.h"


class VuWin32File : public VuGenericWinFile
{
private:
	virtual bool	init(const std::string &rootPath, const std::string &projectName);
};
