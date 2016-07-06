//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PVRTC util
// 
//*****************************************************************************

#include "VuPvrtc.h"


#if defined VUWIN32 && !VU_DISABLE_BAKING

#include "VuEngine/Util/VuImageUtil.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/HAL/Sys/Win32/VuWin32Sys.h"

void VuPvrtc::compressImage(const VUUINT8 *rgba, int width, int height, VuArray<VUBYTE> &output, bool createMipMaps, bool alphaOn, bool assumeTiles)
{
	// convert to argb
	VuArray<VUBYTE> argb(0);
	argb.resize(width*height*4);
	VuImageUtil::convertRGBAtoARGB(rgba, width, height, &argb[0]);

	char tempFileName[MAX_PATH];
	if ( GetTempFileName(".", "", 0, tempFileName) == 0 )
	{
		VUASSERT(0, "Unable to create temp file!");
		return;
	}

	if ( !VuFileUtil::saveFile(tempFileName, &argb[0], width*height*4) )
	{
		VUASSERT(0, "Unable to write temp file!");
		return;
	}

	char cmdLine[256];
	VU_SPRINTF(cmdLine, sizeof(cmdLine), "%s %d %d", tempFileName, width, height);

	if ( createMipMaps) VU_STRCAT(cmdLine, sizeof(cmdLine), " createMipMaps");
	if ( alphaOn)       VU_STRCAT(cmdLine, sizeof(cmdLine), " alphaOn");
	if ( assumeTiles)   VU_STRCAT(cmdLine, sizeof(cmdLine), " assumeTiles");

	if ( !VuWin32Sys::IF()->createProcess("VuPvrtc.exe", cmdLine, VUNULL, true) )
	{
		VUASSERT(0, "Unable to launch VuPvrtc.exe!");
		return;
	}

	// read file
	VuArray<VUBYTE> pvrtc;
	if ( !VuFileUtil::loadFile(tempFileName, pvrtc) )
	{
		VUASSERT(0, "Unable to read temp file!");
		VuFile::IF()->remove(tempFileName);
		return;
	}

	if ( pvrtc.size() == output.size() )
	{
		VU_MEMCPY(&output[0], output.size(), &pvrtc[0], pvrtc.size());
	}
	else
	{
		VUASSERT(0, "Unexpected temp file size!");
	}

	// clean up
	VuFile::IF()->remove(tempFileName);
}

#else

void VuPvrtc::compressImage(const VUUINT8 *rgba, int width, int height, VuArray<VUBYTE> &output, bool createMipMaps, bool alphaOn, bool assumeTiles) {}

#endif