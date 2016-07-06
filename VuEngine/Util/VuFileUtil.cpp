//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  File utility functionality.
// 
//*****************************************************************************

#include "VuFileUtil.h"
#include "VuBinaryDataUtil.h"
#include "VuEngine/HAL/File/VuFile.h"


//*****************************************************************************
std::string VuFileUtil::fixSlashes(const std::string &strPathNameExt)
{
	std::string str = strPathNameExt;

	// convert all \ to /
	for ( int i = 0; i < (int)str.length(); i++ )
		if ( str[i] == '\\' )
			str[i] = '/';

	// convert all // to /
	while ( str.find("//") != std::string::npos )
		str.erase(str.begin() + str.find("//"));

	return str;
}

//*****************************************************************************
std::string VuFileUtil::fixPath(const std::string &strPathNameExt)
{
	std::string str = fixSlashes(strPathNameExt);

	if ( str.length() && *(str.end() - 1) != '/' )
		str += "/";

	return str;
}

//*****************************************************************************
std::string VuFileUtil::getPath(const std::string &strPathNameExt)
{
	std::string cleanPathNameExt = fixSlashes(strPathNameExt);
	size_t slash = (int)cleanPathNameExt.find_last_of('/');
	if ( slash != std::string::npos )
		return strPathNameExt.substr(0, slash);
	return "";
}

//*****************************************************************************
std::string VuFileUtil::getName(const std::string &strPathNameExt)
{
	std::string name = fixSlashes(strPathNameExt);
	size_t slash = (int)name.find_last_of('/');
	if ( slash != std::string::npos )
		name = name.substr(slash + 1);
	size_t dot = (int)name.find_last_of('.');
	if ( dot != std::string::npos )
		name = name.substr(0, dot);

	return name;
}

//*****************************************************************************
std::string VuFileUtil::getExt(const std::string &strPathNameExt)
{
	size_t dot = (int)strPathNameExt.find_last_of('.');
	if ( dot != std::string::npos )
		return strPathNameExt.substr(dot + 1);
	return "";
}

//*****************************************************************************
std::string VuFileUtil::getNameExt(const std::string &strPathNameExt)
{
	std::string cleanPathNameExt = fixSlashes(strPathNameExt);
	size_t slash = (int)cleanPathNameExt.find_last_of('/');
	if ( slash != std::string::npos )
		return strPathNameExt.substr(slash + 1);
	return "";
}

//*****************************************************************************
std::string VuFileUtil::getPathName(const std::string &strPathNameExt)
{
	size_t dot = (int)strPathNameExt.find_last_of('.');
	if ( dot != std::string::npos )
		return strPathNameExt.substr(0, dot);
	return strPathNameExt;
}

//*****************************************************************************
bool VuFileUtil::loadFile(const std::string &strFileName, VuArray<VUBYTE> &data)
{
	VuBinaryDataWriter writer(data);
	return loadFile(strFileName, writer);
}

//*****************************************************************************
bool VuFileUtil::loadFile(const std::string &strFileName, VuBinaryDataWriter &writer)
{
	VUHANDLE hFile = VuFile::IF()->open(strFileName, VuFile::MODE_READ);
	if ( !hFile )
		return false;

	int size = VuFile::IF()->size(hFile);
	void *pData = writer.allocate(size);

	bool bSuccess = VuFile::IF()->read(hFile, pData, size) == size;

	VuFile::IF()->close(hFile);

	return bSuccess;
}

//*****************************************************************************
bool VuFileUtil::saveFile(const std::string &strFileName, const void *pData, VUINT size)
{
	VUHANDLE hFile = VuFile::IF()->open(strFileName, VuFile::MODE_WRITE);
	if ( !hFile )
		return false;

	bool bSuccess = VuFile::IF()->write(hFile, pData, size) == size;

	VuFile::IF()->close(hFile);

	return bSuccess;
}
