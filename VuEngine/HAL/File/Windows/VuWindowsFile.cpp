//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include "VuWindowsFile.h"
#include "VuEngine/Util/VuUtf8.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFile, VuWindowsFile);


//*****************************************************************************
void VuWindowsFile::getLocalFolder(std::string &localFolder)
{
	VuUtf8::convertWCharStringToUtf8String(Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data(), localFolder);
}