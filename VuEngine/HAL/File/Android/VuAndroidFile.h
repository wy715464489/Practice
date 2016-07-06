//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android interface class for File.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/File/Generic/VuGenericFile.h"

struct AAssetManager;


class VuAndroidFile : public VuGenericFile
{
public:
	VuAndroidFile() : mpExpansionFileIF(VUNULL) {}

	virtual bool		init(const std::string &rootPath, const std::string &projectName);

	virtual bool		exists(const std::string &strFileName);

	virtual VUHANDLE	open(const std::string &strFileName, eMode mode);
	virtual bool		close(VUHANDLE hFile);
	virtual int			read(VUHANDLE hFile, void *pData, VUINT size);
	virtual bool		seek(VUHANDLE hFile, int pos);
	virtual int			tell(VUHANDLE hFile);
	virtual int			size(VUHANDLE hFile);

	virtual void		enumFiles(FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard);

	virtual bool		createDirectory(const std::string &strPath);

	// platform-specific functionality
	static VuAndroidFile *IF() { return static_cast<VuAndroidFile *>(VuFile::IF()); }

	static void			setAssetManager(AAssetManager *pAssetManager);
	static void			setInternalDataPath(const char *path);
	void				getFilesPath(std::string &path);

	class ExpansionFileIF
	{
	public:
		virtual VUHANDLE	open() = 0;
		virtual void		close(VUHANDLE hFile) = 0;
		virtual int			read(VUHANDLE hFile, void *pData, int size) = 0;
		virtual bool		seek(VUHANDLE hFile, int pos) = 0;
	};
	void			setExpansionFileIF(ExpansionFileIF *pIF) { mpExpansionFileIF = pIF; }


private:
	bool				isApkPath(const char *strPath);

	ExpansionFileIF		*mpExpansionFileIF;
};
