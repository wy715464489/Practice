//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to write data containers in binary format.
// 
//*****************************************************************************

#pragma once


class VuJsonContainer;


class VuJsonBinaryWriter
{
public:
	VuJsonBinaryWriter();

	bool			saveToFile(const VuJsonContainer &container, const std::string &strFileName);
	bool			saveToMemory(const VuJsonContainer &container, void *pData, int &dataSize);

	static int		calculateDataSize(const VuJsonContainer &container);

private:

	static int		calculateContainerDataSize(const VuJsonContainer &container);
	bool			writeContainer(const VuJsonContainer &container);
	template <class T>
	bool			writeValue(const T &val);
	bool			writeString(const std::string &str);

	unsigned char	*mpDataPtr;
	int				mDataRemaining;
};


#include "VuJsonBinaryWriter.inl"
