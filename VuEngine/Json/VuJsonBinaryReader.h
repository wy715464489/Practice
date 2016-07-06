//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to read data containers in binary format.
// 
//*****************************************************************************

#pragma once


class VuJsonContainer;


class VuJsonBinaryReader
{
public:
	VuJsonBinaryReader();

	bool				loadFromFile(VuJsonContainer &container, const std::string &strFileName);
	bool				loadFromMemory(VuJsonContainer &container, const void *pData, int dataSize);

	const std::string	&getLastError();

private:

	bool				error(const char *fmt, ...);

	bool				readContainer(VuJsonContainer &container);
	template <class T>
	bool				readValue(T &val);
	bool				readString();

	const unsigned char	*mpDataPtr;
	int					mDataRemaining;
	std::vector<char>	mStringBuffer;
	std::string			mstrError;
};


#include "VuJsonBinaryReader.inl"
