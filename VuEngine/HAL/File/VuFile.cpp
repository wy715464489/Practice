//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include "VuFile.h"
#include "VuFileHostIO.h"
#include "VuEngine/Util/VuFileUtil.h"


//*****************************************************************************
VuFile::VuFile():
	mpHostIO(VUNULL)
{
}

//*****************************************************************************
bool VuFile::init(const std::string &rootPath, const std::string &projectName)
{
	mRootPath = VuFileUtil::fixPath(rootPath);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( VuFileHostIO::isHostPath(rootPath.c_str()) )
		mpHostIO = new VuFileHostIO;
#endif

	return true;
}

//*****************************************************************************
void VuFile::release()
{
	delete mpHostIO;
}