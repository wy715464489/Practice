//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include <android/asset_manager.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "VuAndroidFile.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFile, VuAndroidFile);


static AAssetManager *spAssetManager = VUNULL;
static std::string sInternalDataPath;

static const std::string sApkRootPath = "apk:/";


//*****************************************************************************
bool VuAndroidFile::init(const std::string &rootPath, const std::string &projectName)
{
	if ( !VuGenericFile::init(rootPath, projectName) )
		return false;

	if ( projectName.length() )
	{
		mCachePath = std::string("/sdcard/") + projectName;
		if ( (mkdir(mCachePath.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == -1) && (errno != EEXIST) )
		{
			return VUERROR("Unable to create cache directory '%s' (errno %d).", mCachePath.c_str(), errno);
		}

		mCachePath += "/";
	}

	return true;
}

//*****************************************************************************
bool VuAndroidFile::exists(const std::string &strFileName)
{
	if ( isApkPath(strFileName.c_str()) )
	{
		const char *apkFileName = strFileName.c_str() + sApkRootPath.length();

		AAsset *pAsset = AAssetManager_open(spAssetManager, apkFileName, AASSET_MODE_UNKNOWN);
		if ( pAsset )
		{
			AAsset_close(pAsset);
			return true;
		}
		return false;
	}

	return VuGenericFile::exists(strFileName);
}

//*****************************************************************************
VUHANDLE VuAndroidFile::open(const std::string &strFileName, eMode mode)
{
	if ( isApkPath(strFileName.c_str()) )
	{
		const char *apkFileName = strFileName.c_str() + sApkRootPath.length();

		if ( mode == MODE_READ )
		{
			AAsset *pAsset = AAssetManager_open(spAssetManager, apkFileName, AASSET_MODE_UNKNOWN);
			if ( pAsset == VUNULL )
			{
				// if the expansion file is not found in assets, try loading it from a platform-specific location
				if ( strcmp(apkFileName, "Expansion.apf") == 0 )
				{
					if ( mpExpansionFileIF )
					{
						if ( VUHANDLE hFile = mpExpansionFileIF->open() )
						{
							VuOpenFile *pOpenFile = new VuOpenFile;
							pOpenFile->mExpansionHandle = hFile;
							pOpenFile->mName = apkFileName;
							return pOpenFile;
						}
					}
				}

				return VUNULL;
			}

			VuOpenFile *pOpenFile = new VuOpenFile;
			pOpenFile->mPlatformHandle = pAsset;
			pOpenFile->mName = apkFileName;
			return pOpenFile;
		}
		return VUNULL;
	}

	return VuGenericFile::open(strFileName, mode);
}

//*****************************************************************************
bool VuAndroidFile::close(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

	if ( pOpenFile->mExpansionHandle )
	{
		mpExpansionFileIF->close(pOpenFile->mExpansionHandle);
		delete pOpenFile;
		return true;
	}
	else if ( pOpenFile->mPlatformHandle )
	{
		AAsset *pAsset = static_cast<AAsset *>(pOpenFile->mPlatformHandle);
		delete pOpenFile;
		AAsset_close(pAsset);
		return true;
	}

	return VuGenericFile::close(hFile);
}

//*****************************************************************************
int VuAndroidFile::read(VUHANDLE hFile, void *pData, VUINT size)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

	if ( pOpenFile->mExpansionHandle )
		return mpExpansionFileIF->read(pOpenFile->mExpansionHandle, pData, size);
	else if ( pOpenFile->mPlatformHandle )
		return AAsset_read(static_cast<AAsset *>(pOpenFile->mPlatformHandle), pData, size);

	return VuGenericFile::read(hFile, pData, size);
}

//*****************************************************************************
bool VuAndroidFile::seek(VUHANDLE hFile, int pos)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

	if ( pOpenFile->mExpansionHandle )
		return mpExpansionFileIF->seek(pOpenFile->mExpansionHandle, pos);
	else if ( pOpenFile->mPlatformHandle )
		return AAsset_seek(static_cast<AAsset *>(pOpenFile->mPlatformHandle), pos, SEEK_SET) != -1;

	return VuGenericFile::seek(hFile, pos);
}

//*****************************************************************************
int VuAndroidFile::tell(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

	if ( pOpenFile->mExpansionHandle )
	{
		VUASSERT(0, "Not supported!");
		return -1;
	}
	else if ( pOpenFile->mPlatformHandle )
	{
		VUASSERT(0, "Not supported!");
		return -1;
	}

	return VuGenericFile::tell(hFile);
}

//*****************************************************************************
int VuAndroidFile::size(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

	if ( pOpenFile->mExpansionHandle )
	{
		VUASSERT(0, "Not supported!");
		return -1;
	}
	else if ( pOpenFile->mPlatformHandle )
	{
		VUASSERT(0, "Not supported!");
		return -1;
	}

	return VuGenericFile::size(hFile);
}

//*****************************************************************************
void VuAndroidFile::enumFiles(VuFile::FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strSearchPath.c_str()) )
		mpHostIO->enumFiles(fileList, strSearchPath.c_str(), strWildCard.c_str());
#endif

	// build search path
	std::string strSearch = VuFileUtil::fixSlashes(strSearchPath + "/" + strWildCard);

	if ( DIR *pDir = opendir(strSearch.c_str()) )
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
bool VuAndroidFile::createDirectory(const std::string &strPath)
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

	if ( mkdir(cleanPath.c_str(), S_IRWXU| S_IRWXG | S_IRWXO) == -1 )
	{
		if ( errno != EEXIST )
			return false;
	}

	return true;
}

//*****************************************************************************
void VuAndroidFile::getFilesPath(std::string &path)
{
	path = sInternalDataPath;
}

//*****************************************************************************
void VuAndroidFile::setAssetManager(AAssetManager *pAssetManager)
{
	spAssetManager = pAssetManager;
}

//*****************************************************************************
void VuAndroidFile::setInternalDataPath(const char *path)
{
	sInternalDataPath = path;
}

//*****************************************************************************
bool VuAndroidFile::isApkPath(const char *strPath)
{
	return strncmp(strPath, sApkRootPath.c_str(), sApkRootPath.length()) == 0;
}