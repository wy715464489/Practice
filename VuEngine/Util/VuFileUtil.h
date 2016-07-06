//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  File utility functionality.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"

class VuBinaryDataWriter;


namespace VuFileUtil
{
	std::string	fixSlashes(const std::string &strPathNameExt);
	std::string	fixPath(const std::string &strPathNameExt);

	std::string	getPath(const std::string &strPathNameExt);
	std::string	getName(const std::string &strPathNameExt);
	std::string	getExt(const std::string &strPathNameExt);
	std::string	getNameExt(const std::string &strPathNameExt);
	std::string	getPathName(const std::string &strPathNameExt);

	// simple i/o
	bool		loadFile(const std::string &strFileName, VuArray<VUBYTE> &data);
	bool		loadFile(const std::string &strFileName, VuBinaryDataWriter &writer);
	bool		saveFile(const std::string &strFileName, const void *pData, VUINT size);
}
