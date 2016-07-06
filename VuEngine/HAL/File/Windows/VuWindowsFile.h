//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class for File.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/File/GenericWin/VuGenericWinFile.h"


class VuWindowsFile : public VuGenericWinFile
{
public:
	static void		getLocalFolder(std::string &localFolder);
};
