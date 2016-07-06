//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include <kernel.h>
#include "VuEngine/HAL/File/Generic/VuGenericFile.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"
#include "VuEngine/Util/VuFileUtil.h"

#define KERNEL_FILES

//*****************************************************************************
class VuPs4File : public VuGenericFile
{
private:
	virtual bool		init(const std::string &rootPath, const std::string &projectName);

#ifdef KERNEL_FILES
	virtual VUHANDLE	open(const std::string &strFileName, eMode mode);
	virtual bool		close(VUHANDLE hFile);
	virtual int			read(VUHANDLE hFile, void *pData, VUINT size);
	virtual int			write(VUHANDLE hFile, const void *pData, VUINT size);
	virtual bool		seek(VUHANDLE hFile, int pos);
	virtual int			tell(VUHANDLE hFile);
	virtual int			size(VUHANDLE hFile);
#endif

public:
	
	virtual void		enumFiles(FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard);
	
	virtual bool		createDirectory(const std::string &strPath);
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFile, VuPs4File);


//*****************************************************************************
bool VuPs4File::init(const std::string &rootPath, const std::string &projectName)
{
	if ( !VuGenericFile::init(rootPath, projectName) )
		return false;

	if ( projectName.length() )
	{
		mCachePath = "/data/" + projectName;
		if ( mkdir(mCachePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1 )
		{
			if ( errno != EEXIST )
				return false;
		}
		mCachePath += "/";
	}
	
	return true;
}

//*****************************************************************************
void VuPs4File::enumFiles(VuFile::FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strSearchPath.c_str()) )
		mpHostIO->enumFiles(fileList, strSearchPath.c_str(), strWildCard.c_str());
#endif

	VUASSERT(0, "This is not expected!");
}

//*****************************************************************************
bool VuPs4File::createDirectory(const std::string &strPath)
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
	
	if ( mkdir(strPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1 )
	{
		if ( errno != EEXIST )
			return false;
	}
	
	return true;
}

#ifdef KERNEL_FILES
//*****************************************************************************
VUHANDLE VuPs4File::open(const std::string &strFileName, eMode mode)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
	{
		VUHANDLE hHostHandle = mpHostIO->open(strFileName.c_str(), mode);
		if ( hHostHandle == VUNULL )
			return VUNULL;
		
		VuOpenFile *pOpenFile = new VuOpenFile;
		pOpenFile->mHostHandle = hHostHandle;
		pOpenFile->mName = strFileName;

		return pOpenFile;
	}
#endif

	VUINT fd = 0;

	if (mode == MODE_READ)
	{
		fd = sceKernelOpen(strFileName.c_str(), SCE_KERNEL_O_RDONLY, SCE_KERNEL_S_INONE);
	}
	else if (mode == MODE_WRITE)
	{
		fd = sceKernelOpen(strFileName.c_str(), SCE_KERNEL_O_RDWR | SCE_KERNEL_O_TRUNC | SCE_KERNEL_O_CREAT, SCE_KERNEL_S_IRWU);
	}
	else if (mode == MODE_READ_WRITE)
	{
		fd = sceKernelOpen(strFileName.c_str(), SCE_KERNEL_O_RDWR, SCE_KERNEL_S_INONE);
	}

	if (fd < SCE_OK)
	{
		// Silently fail when trying to open a file that's not there
		if (fd != SCE_KERNEL_ERROR_ENOENT)
		{
			VUPRINTF("sceKernelOpen : 0x%08x(%s)\n", fd, strFileName.c_str());
		}

		return (VUHANDLE)0;
	}

	VuOpenFile *pOpenFile = new VuOpenFile;
	pOpenFile->mpFile = VUNULL;
	pOpenFile->mName = strFileName;
	pOpenFile->mPlatformHandle =(VUHANDLE)(VUUINT64)fd;

	return pOpenFile;
}


//*****************************************************************************
bool VuPs4File::close(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if (pOpenFile->mHostHandle)
	{
		VUHANDLE hHostHandle = pOpenFile->mHostHandle;
		delete pOpenFile;
		return mpHostIO->close(hHostHandle);
	}
#endif

	VUINT fd = static_cast<VUINT>((VUUINT64)pOpenFile->mPlatformHandle);
	delete pOpenFile;

	VUINT result = sceKernelClose(fd);
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceKernelClose(): Error=%08x\n", result);

		return false;
	}

	return true;
}

//*****************************************************************************
int VuPs4File::read(VUHANDLE hFile, void *pData, VUINT size)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if (pOpenFile->mHostHandle)
		return mpHostIO->read(pOpenFile->mHostHandle, pData, size);
#endif

	VUINT fd = static_cast<VUINT>((VUUINT64)pOpenFile->mPlatformHandle);

	VUINT result = static_cast<int32_t>(sceKernelRead(fd, pData, static_cast<size_t>(size)));
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceKernelRead(): Error=%08x\n", result);
		sceKernelClose(fd);
		return 0;
	}

	return result;
}

//*****************************************************************************
int VuPs4File::write(VUHANDLE hFile, const void *pData, VUINT size)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if (pOpenFile->mHostHandle)
		return mpHostIO->write(pOpenFile->mHostHandle, pData, size);
#endif

	VUINT fd = static_cast<VUINT>((VUUINT64)pOpenFile->mPlatformHandle);

	VUINT result = static_cast<int32_t>(sceKernelWrite(fd, pData, static_cast<size_t>(size)));
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceKernelWrite(): Error=%08x\n", result);
		sceKernelClose(fd);
		return 0;
	}

	return result;
}

//*****************************************************************************
bool VuPs4File::seek(VUHANDLE hFile, int pos)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if (pOpenFile->mHostHandle)
		return mpHostIO->seek(pOpenFile->mHostHandle, pos);
#endif

	VUINT fd = static_cast<VUINT>((VUUINT64)pOpenFile->mPlatformHandle);

	VUINT result = static_cast<VUINT>(sceKernelLseek(fd, pos, SCE_KERNEL_SEEK_SET));
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceKernelLseek(): Error=%08x\n", result);

		return false;
	}

	return true;
}

//*****************************************************************************
int VuPs4File::tell(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if (pOpenFile->mHostHandle)
		return mpHostIO->tell(pOpenFile->mHostHandle);
#endif

	VUINT fd = static_cast<VUINT>((VUUINT64)pOpenFile->mPlatformHandle);

	// PS4 doesn't have ftell() equivalent, so we seek 0 bytes from the current position and return
	// that offset
	VUINT result = static_cast<VUINT>(sceKernelLseek(fd, 0, SCE_KERNEL_SEEK_CUR));
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceKernelLseek(): Error=%08x\n", result);

		return 0;
	}

	return result;
}

//*****************************************************************************
int VuPs4File::size(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if (pOpenFile->mHostHandle)
		return mpHostIO->size(pOpenFile->mHostHandle);
#endif

	VUINT fd = static_cast<VUINT>((VUUINT64)pOpenFile->mPlatformHandle);

	SceKernelStat status;

	VUINT result = sceKernelFstat(fd, &status);
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceKernelFstat(): Error=%08x\n", result);

		return 0;
	}

	return status.st_size;
}
#endif // KERNEL_FILES