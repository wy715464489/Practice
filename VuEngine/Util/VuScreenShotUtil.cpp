//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Screen shot utility
// 
//*****************************************************************************

#include "VuScreenShotUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuTgaUtil.h"


#define SCREEN_SHOT_FOLDER "ScreenShots"


//*****************************************************************************
VuScreenShotWriter::VuScreenShotWriter(int width, int height):
	mhFile(VUNULL)
{
	// create directory
	VuFile::IF()->createDirectory(VuFile::IF()->getRootPath() + SCREEN_SHOT_FOLDER);

	// determine file name
	std::string fileName;
	int i = 1;
	do
	{
		char str[256];
		VU_SPRINTF(str, sizeof(str), VUPLATFORM "_%04d.tga", i);
		fileName = VuFile::IF()->getRootPath() + SCREEN_SHOT_FOLDER + "/" + str;
		i++;
	} while ( VuFile::IF()->exists(fileName) );

	// open file
	mhFile = VuFile::IF()->open(fileName, VuFile::MODE_WRITE);

	// write header
	if ( mhFile )
	{
		VuArray<VUBYTE> header(0);
		VuTgaUtil::createHeader(24, width, height, true, header);
		VuFile::IF()->write(mhFile, &header[0], header.size());
	}
}

//*****************************************************************************
VuScreenShotWriter::~VuScreenShotWriter()
{
	if ( mhFile )
		VuFile::IF()->close(mhFile);
}

//*****************************************************************************
void VuScreenShotWriter::write(const VUBYTE *bgr, int size)
{
	if ( mhFile )
		VuFile::IF()->write(mhFile, bgr, size);
}
