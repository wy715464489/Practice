//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset dependencies class
// 
//*****************************************************************************

#include "VuAssetDependencies.h"
#include "VuAssetFactory.h"
#include "VuAssetBakery.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
void VuAssetDependencies::addAsset(const std::string &type, const std::string &name)
{
	for ( AssetEntries::const_iterator iter = mAssetEntries.begin(); iter != mAssetEntries.end(); iter++ )
		if ( iter->mType == type && iter->mName == name )
			return;

	VuAssetEntry entry;
	entry.mType = type;
	entry.mName = name;
	entry.mInfoHash = 0;
	entry.mDataHash = 0;
	mAssetEntries.push_back(entry);
}

//*****************************************************************************
void VuAssetDependencies::addFile(const std::string &fileName)
{
	for ( FileEntries::const_iterator iter = mFileEntries.begin(); iter != mFileEntries.end(); iter++ )
		if ( iter->mFileName == fileName )
			return;

	VuFileEntry entry;
	entry.mFileName = fileName;
	entry.mHash = 0;
	mFileEntries.push_back(entry);
}

//*****************************************************************************
bool VuAssetDependencies::finalize(const std::string &platform, const std::string &sku, const std::string &language)
{
	for ( AssetEntries::iterator iter = mAssetEntries.begin(); iter != mAssetEntries.end(); iter++ )
	{
		iter->mInfoHash = VuAssetBakery::IF()->getAssetInfoHash(iter->mType, iter->mName);
		if ( !VuAssetBakery::IF()->getAssetDataHash(platform, sku, language,  iter->mType, iter->mName, iter->mDataHash) )
			return false;
	}

	for ( FileEntries::iterator iter = mFileEntries.begin(); iter != mFileEntries.end(); iter++ )
	{
		iter->mHash = VuFile::IF()->hash32(VuFile::IF()->getRootPath() + iter->mFileName);
	}

	return true;
}

//*****************************************************************************
void VuAssetDependencies::serialize(VuBinaryDataWriter &writer) const
{
	int assetCount = (int)mAssetEntries.size();
	writer.writeValue(assetCount);

	for ( AssetEntries::const_iterator iter = mAssetEntries.begin(); iter != mAssetEntries.end(); iter++ )
	{
		writer.writeString(iter->mType);
		writer.writeString(iter->mName);
		writer.writeValue(iter->mInfoHash);
		writer.writeValue(iter->mDataHash);
	}

	int fileCount = (int)mFileEntries.size();
	writer.writeValue(fileCount);

	for ( FileEntries::const_iterator iter = mFileEntries.begin(); iter != mFileEntries.end(); iter++ )
	{
		writer.writeString(iter->mFileName);
		writer.writeValue(iter->mHash);
	}
}

//*****************************************************************************
void VuAssetDependencies::deserialize(VuBinaryDataReader &reader)
{
	int assetCount;
	reader.readValue(assetCount);
	bool swapEndian = assetCount > 65536;

	if ( swapEndian )
		VuEndianUtil::swapInPlace(assetCount);

	mAssetEntries.resize(assetCount);

	for ( AssetEntries::iterator iter = mAssetEntries.begin(); iter != mAssetEntries.end(); iter++ )
	{
		reader.readString(iter->mType);
		reader.readString(iter->mName);
		reader.readValue(iter->mInfoHash);
		reader.readValue(iter->mDataHash);

		if ( swapEndian )
		{
			VuEndianUtil::swapInPlace(iter->mInfoHash);
			VuEndianUtil::swapInPlace(iter->mDataHash);
		}
	}


	int fileCount;
	reader.readValue(fileCount);
	swapEndian = fileCount > 65536;

	if ( swapEndian )
		VuEndianUtil::swapInPlace(fileCount);

	mFileEntries.resize(fileCount);

	for ( FileEntries::iterator iter = mFileEntries.begin(); iter != mFileEntries.end(); iter++ )
	{
		reader.readString(iter->mFileName);
		reader.readValue(iter->mHash);

		if ( swapEndian )
			VuEndianUtil::swapInPlace(iter->mHash);
	}
}

//*****************************************************************************
bool VuAssetDependencies::check(const std::string &language)
{
	for ( AssetEntries::iterator iter = mAssetEntries.begin(); iter != mAssetEntries.end(); iter++ )
	{
		VUUINT32 infoHash = VuAssetFactory::IF()->getAssetInfoHash(iter->mType, iter->mName);
		if ( iter->mInfoHash != infoHash )
		{
			VUPRINTF("Asset info dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}

		VuBakedAssetHeader header;
		VuArray<VUBYTE> data(0);
		VuAssetDependencies dependencies;
		if ( !VuAssetBakery::loadBakedFile(VUPLATFORM, VuAssetFactory::IF()->getSku(), iter->mType, iter->mName, language, infoHash, header, dependencies, data, VuAssetBakery::LBFM_HEADER) )
		{
			VUPRINTF("Asset data dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}

		if ( header.mDataHash != iter->mDataHash )
		{
			VUPRINTF("Asset data dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}

		if ( !dependencies.check(language) )
		{
			VUPRINTF("Asset data dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}
	}

	for ( FileEntries::iterator iter = mFileEntries.begin(); iter != mFileEntries.end(); iter++ )
	{
		if ( iter->mHash != VuFile::IF()->hash32(VuFile::IF()->getRootPath() + iter->mFileName) )
		{
			VUPRINTF("File dependency out of date: %s\n", iter->mFileName.c_str());
			return false;
		}
	}

	return true;
}

//*****************************************************************************
bool VuAssetDependencies::check(const std::string &platform, const std::string &sku, const std::string &language)
{
	for ( AssetEntries::iterator iter = mAssetEntries.begin(); iter != mAssetEntries.end(); iter++ )
	{
		VUUINT32 infoHash = VuAssetBakery::IF()->getAssetInfoHash(iter->mType, iter->mName);
		if ( iter->mInfoHash != infoHash )
		{
			VUPRINTF("Asset info dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}

		VuBakedAssetHeader header;
		VuArray<VUBYTE> data(0);
		VuAssetDependencies dependencies;
		if ( !VuAssetBakery::loadBakedFile(platform, sku, iter->mType, iter->mName, language, infoHash, header, dependencies, data, VuAssetBakery::LBFM_HEADER) )
		{
			VUPRINTF("Asset data dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}

		if ( header.mDataHash != iter->mDataHash )
		{
			VUPRINTF("Asset data dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}

		if ( !dependencies.check(platform, sku, language) )
		{
			VUPRINTF("Asset data dependency out of date: %s.%s\n", iter->mType.c_str(), iter->mName.c_str());
			return false;
		}
	}

	for ( FileEntries::iterator iter = mFileEntries.begin(); iter != mFileEntries.end(); iter++ )
	{
		if ( iter->mHash != VuFile::IF()->hash32(VuFile::IF()->getRootPath() + iter->mFileName) )
		{
			VUPRINTF("File dependency out of date: %s\n", iter->mFileName.c_str());
			return false;
		}
	}

	return true;
}
