//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Pack File class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"


class VuAssetPackFileBase
{
public:
	VuAssetPackFileBase() : mVersion(-1) {}

	const std::string	&getSku()				{ return mSku; }
	int					getVersion()			{ return mVersion; }

	bool				dumpStats(const std::string &strStatsFileName);

	struct Entry
	{
		VUUINT32		mOffset;
		VUUINT32		mUncompressedSize;
		VUUINT32		mCompressedSize;
		VUUINT32		mHash;
		VUUINT16		mVersion;
		VUUINT16		mCompressionType;
	};

protected:
	typedef std::map<std::string, Entry> Entries;

	std::string	mSku;
	int			mVersion;
	Entries		mEntries;
};

class VuAssetPackFileReader : public VuAssetPackFileBase
{
public:
	VuAssetPackFileReader();
	~VuAssetPackFileReader();

	enum eSeekResult { RESULT_NOT_FOUND, RESULT_SUCCESS, RESULT_ERROR };

	bool		open(const std::string &strPackFileName);
	eSeekResult	seek(const std::string &strAssetType, const std::string &strAssetName, const std::string &strLang, Entry &entry);
	VUHANDLE	handle() { return mhFile; }
	bool		close();

private:
	VUHANDLE	mhFile;
};

class VuAssetPackFileWriter : public VuAssetPackFileBase
{
public:
	VuAssetPackFileWriter();
	~VuAssetPackFileWriter();

	bool		open(const std::string &strPackFileName, const std::string &sku, int version);
	bool		write(const char *strAssetType, const std::string &strAssetName, const std::string &strLang, VUUINT32 version, VUUINT32 hash, VUUINT32 uncompressedSize, VuArray<VUBYTE> &compressedData, VUUINT32 compressionType);
	bool		close(const std::string &platform);

private:
	VUHANDLE	mhFile;
};
