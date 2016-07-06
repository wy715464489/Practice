//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Pack File class
// 
//*****************************************************************************

#include "VuAssetPackFile.h"
#include "VuEngine/Entities/VuEntityUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


#define MIN_FILE_NAME_LENGTH 8


struct AssetPackFileHeader
{
	VUUINT32	mMagic;
	VUUINT32	mHeaderVersion;
	VUUINT32	mDataSize;
	VUUINT32	mEntryCount;
	VUUINT32	mTocSize;
	VUUINT32	mTocHash;
	char		mSku[32];
	int			mVersion;

	// this must be last!!
	VUUINT32	mHeaderHash;
};

static const VUUINT32 scMagic = ('V'<<24)|('U'<<16)|('P'<<8)|('F');
static const VUUINT32 scHeaderVersion = 5;


//*****************************************************************************
VuAssetPackFileReader::VuAssetPackFileReader():
	mhFile(VUNULL)
{
}

//*****************************************************************************
VuAssetPackFileReader::~VuAssetPackFileReader()
{
	close();
}

//*****************************************************************************
bool VuAssetPackFileReader::open(const std::string &strPackFileName)
{
	// already open?
	if ( mhFile != VUNULL )
		return false;

	mhFile = VuFile::IF()->open(strPackFileName, VuFile::MODE_READ);
	if ( mhFile == VUNULL )
		return false;

	// read header
	AssetPackFileHeader header;
	if ( VuFile::IF()->read(mhFile, &header, sizeof(header)) != sizeof(header) )
	{
		VuFile::IF()->close(mhFile);
		mhFile = VUNULL;
		return false;
	}
	mSku = header.mSku;
	mVersion = header.mVersion;

	// verify header
	if ( header.mMagic != scMagic || header.mHeaderVersion != scHeaderVersion || header.mHeaderHash != VuHash::fnv32(&header, sizeof(header) - 4) )
	{
		VuFile::IF()->close(mhFile);
		mhFile = VUNULL;
		return false;
	}

	// read TOC data
	if ( !VuFile::IF()->seek(mhFile, header.mDataSize) )
	{
		VuFile::IF()->close(mhFile);
		mhFile = VUNULL;
		return false;
	}

	VuArray<VUBYTE> tocData(0);
	tocData.resize(header.mTocSize);
	if ( VuFile::IF()->read(mhFile, &tocData.begin(), tocData.size()) != tocData.size() )
	{
		VuFile::IF()->close(mhFile);
		mhFile = VUNULL;
		return false;
	}

	// verify TOC data
	#if VERIFY_PACKED_ASSET_HASH
		if ( header.mTocHash != VuHash::fnv32(&tocData.begin(), tocData.size()) )
			return false;
	#endif

	VuBinaryDataReader reader(tocData);
	for ( int i = 0; i < (int)header.mEntryCount; i++ )
	{
		if ( reader.remaining() < sizeof(Entry) + MIN_FILE_NAME_LENGTH )
		{
			VuFile::IF()->close(mhFile);
			mhFile = VUNULL;
			return false;
		}

		const char *strAssetFileName = reader.readString();

		Entry &entry = mEntries[strAssetFileName];
		reader.readValue(entry.mOffset);
		reader.readValue(entry.mUncompressedSize);
		reader.readValue(entry.mCompressedSize);
		reader.readValue(entry.mHash);
		reader.readValue(entry.mVersion);
		reader.readValue(entry.mCompressionType);
	}

	return true;
}

//*****************************************************************************
VuAssetPackFileReader::eSeekResult VuAssetPackFileReader::seek(const std::string &strAssetType, const std::string &strAssetName, const std::string &strLang, Entry &entry)
{
	if ( mhFile == VUNULL )
		return RESULT_NOT_FOUND;

	std::string fullName = strAssetType + "/" + strAssetName + strLang;

	Entries::const_iterator itEntry = mEntries.find(fullName);
	if ( itEntry == mEntries.end() )
		return RESULT_NOT_FOUND;

	entry = itEntry->second;

	if ( !VuFile::IF()->seek(mhFile, entry.mOffset) )
		return RESULT_ERROR;

	return RESULT_SUCCESS;
}

//*****************************************************************************
bool VuAssetPackFileReader::close()
{
	mEntries.clear();

	if ( mhFile )
	{
		VuFile::IF()->close(mhFile);
		mhFile = VUNULL;
	}

	return true;
}

//*****************************************************************************
VuAssetPackFileWriter::VuAssetPackFileWriter():
	mhFile(VUNULL)
{
}

//*****************************************************************************
VuAssetPackFileWriter::~VuAssetPackFileWriter()
{
	if ( mhFile )
	{
		VuFile::IF()->close(mhFile);
		mEntries.clear();
	}
}

//*****************************************************************************
bool VuAssetPackFileWriter::open(const std::string &strPackFileName, const std::string &sku, int version)
{
	if ( mhFile )
		return false;

	mhFile = VuFile::IF()->open(strPackFileName, VuFile::MODE_WRITE);
	if ( mhFile == VUNULL )
		return false;

	// write empty header
	AssetPackFileHeader header;
	memset(&header, 0, sizeof(header));
	if ( VuFile::IF()->write(mhFile, &header, sizeof(header)) != sizeof(header) )
		return false;

	if ( sku.length() >= sizeof(header.mSku) )
		return false;

	mSku = sku;
	mVersion = version;

	return true;
}

//*****************************************************************************
bool VuAssetPackFileWriter::write(const char *strAssetType, const std::string &strAssetName, const std::string &strLang, VUUINT32 version, VUUINT32 hash, VUUINT32 uncompressedSize, VuArray<VUBYTE> &compressedData, VUUINT32 compressionType)
{
	if ( mhFile == VUNULL )
		return false;

	int offsetStart = VuFile::IF()->tell(mhFile);

	// write data
	if (VuFile::IF()->write(mhFile, &compressedData.begin(), compressedData.size()) != compressedData.size())
		return false;

	// add entry
	Entry entry;
	entry.mOffset = offsetStart;
	entry.mUncompressedSize = uncompressedSize;
	entry.mCompressedSize = compressedData.size();
	entry.mHash = hash;
	entry.mVersion = (VUUINT16)version;
	entry.mCompressionType = (VUUINT16)compressionType;

	mEntries[std::string(strAssetType) + "/" + strAssetName + strLang] = entry;

	return true;
}

//*****************************************************************************
bool VuAssetPackFileWriter::close(const std::string &platform)
{
	if ( mhFile )
	{
		int totalDataSize = VuFile::IF()->tell(mhFile);

		// create TOC data
		VuArray<VUBYTE> tocData;
		{
			VuBinaryDataWriter writer(tocData);
			writer.configure(platform);
			for ( Entries::const_iterator iter = mEntries.begin(); iter != mEntries.end(); iter++ )
			{
				writer.writeString(iter->first);
				writer.writeValue(iter->second.mOffset);
				writer.writeValue(iter->second.mUncompressedSize);
				writer.writeValue(iter->second.mCompressedSize);
				writer.writeValue(iter->second.mHash);
				writer.writeValue(iter->second.mVersion);
				writer.writeValue(iter->second.mCompressionType);
			}
		}

		// create header data
		VuArray<VUBYTE> headerData;
		{
			VuBinaryDataWriter writer(headerData);
			writer.configure(platform);

			AssetPackFileHeader header;
			memset(&header, 0, sizeof(header));

			header.mMagic = scMagic;
			header.mHeaderVersion = scHeaderVersion;
			header.mDataSize = totalDataSize;
			header.mEntryCount = (VUUINT32)mEntries.size();
			header.mTocSize = tocData.size();
			header.mTocHash = VuHash::fnv32(&tocData.begin(), tocData.size());
			VU_STRNCPY(header.mSku, sizeof(header.mSku), mSku.c_str(), sizeof(header.mSku) - 1);
			header.mVersion = mVersion;
			header.mHeaderHash = VuHash::fnv32(&header, sizeof(header) - 4);

			writer.writeValue(header.mMagic);
			writer.writeValue(header.mHeaderVersion);
			writer.writeValue(header.mDataSize);
			writer.writeValue(header.mEntryCount);
			writer.writeValue(header.mTocSize);
			writer.writeValue(header.mTocHash);
			writer.writeData(header.mSku, sizeof(header.mSku));
			writer.writeValue(header.mVersion);
			writer.writeValue(header.mHeaderHash);
		}

		// write header
		if ( !VuFile::IF()->seek(mhFile, 0) )
			return false;
		if ( VuFile::IF()->write(mhFile, &headerData[0], headerData.size()) != headerData.size() )
			return false;

		// write TOC
		if ( !VuFile::IF()->seek(mhFile, totalDataSize) )
			return false;

		if ( VuFile::IF()->write(mhFile, &tocData[0], tocData.size()) != tocData.size() )
			return false;

		// clean up
		if ( !VuFile::IF()->close(mhFile) )
			return false;

		mhFile = VUNULL;
		mEntries.clear();
	}

	return true;
}

//*****************************************************************************
bool VuAssetPackFileBase::dumpStats(const std::string &strStatsFileName)
{
	FILE *fp;
	if ( fopen_s(&fp, strStatsFileName.c_str(), "wt") != 0 )
		return false;

	for ( Entries::const_iterator iter = mEntries.begin(); iter != mEntries.end(); iter++ )
	{
		std::string type = VuEntityUtil::getRoot(iter->first);
		std::string name = VuEntityUtil::subtractRoot(iter->first);
		fprintf(fp, "%s, %s, %d, %d\n", type.c_str(), name.c_str(), iter->second.mUncompressedSize, iter->second.mCompressedSize);
	}

	fclose(fp);

	return true;
}
