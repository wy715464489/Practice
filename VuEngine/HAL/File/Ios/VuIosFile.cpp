//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "VuIosFile.h"
#include "VuIosFileObjC.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFile, VuIosFile);


//*****************************************************************************
bool VuIosFile::init(const std::string &rootPath, const std::string &projectName)
{
	if ( !VuGenericFile::init(rootPath, projectName) )
		return false;
	
	// get document path
	{
		char strDocumentPath[256];
		if ( !VuIosFileObjC::GetDocumentPath(strDocumentPath, sizeof(strDocumentPath)) )
			return false;
		mDocumentPath = strDocumentPath;
	}
	
	if ( projectName.length() )
	{
		mCachePath = getDocumentPath() + "/";
	}
	
	return true;
}

//*****************************************************************************
void VuIosFile::enumFiles(VuFile::FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strSearchPath.c_str()) )
		mpHostIO->enumFiles(fileList, strSearchPath.c_str(), strWildCard.c_str());
#endif
	
	if ( DIR *pDir = opendir(strSearchPath.c_str()) )
	{
		std::string nameWildCard = VuFileUtil::getName(strWildCard);
		std::string extWildCard = VuFileUtil::getExt(strWildCard);
		
		dirent *pDirEnt;
		while ( (pDirEnt = readdir(pDir)) != NULL )
		{
			if ( nameWildCard != "*" )
				if ( VuFileUtil::getName(pDirEnt->d_name) != nameWildCard )
					continue;
			
			if ( extWildCard != "*" )
				if ( VuFileUtil::getExt(pDirEnt->d_name) != extWildCard )
					continue;
			
			fileList.push_back(pDirEnt->d_name);
		}
		
		closedir(pDir);
	}
}

//*****************************************************************************
bool VuIosFile::createDirectory(const std::string &strPath)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strPath.c_str()) )
		return mpHostIO->createDirectory(strPath.c_str());
#endif
	
	std::string cleanPath = VuFileUtil::fixSlashes(strPath);
	while ( cleanPath.length() && cleanPath[cleanPath.length() - 1] == '/' )
		cleanPath.resize(cleanPath.length() - 1);
	
	if ( exists(cleanPath) )
		return true;
	
	std::string parentPath = VuFileUtil::getPath(cleanPath);
	if ( parentPath.length() )
		if ( !createDirectory(parentPath) )
			return false;
	
	if ( mkdir(cleanPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1 )
	{
		if ( errno != EEXIST )
			return false;
	}
	
	return true;
}
