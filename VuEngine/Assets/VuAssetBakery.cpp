//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Baker class
// 
//*****************************************************************************

#include "VuAssetBakery.h"
#include "VuAssetFactory.h"
#include "VuAssetPackFile.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuZLibUtil.h"
#include "VuEngine/Util/VuLzma.h"
#include "VuEngine/Util/VuFilterExpression.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAssetBakery, VuAssetBakery);


#define BAKED_ASSETS_FOLDER "BakedAssets"

static const VUUINT32 scBakedAssetMagic = ('V'<<24)|('U'<<16)|('B'<<8)|('A');
static const VUUINT32 scBakedAssetHeaderVersion = 20;

struct VuAssetDefinitionEntry
{
	VUUINT32	mLanguageOverrides;
};


//*****************************************************************************
bool VuAssetBakery::init()
{
	if ( !reloadAssetInfo() )
		return false;

	return true;
}

//*****************************************************************************
bool VuAssetBakery::reloadAssetInfo()
{
	mAssetDB.clear();

	// load TOC
	VuJsonContainer tocData;
	VuJsonReader reader;
	if ( !reader.loadFromFile(tocData, VuFile::IF()->getRootPath() + VuAssetFactory::IF()->getDevPackageInfo()["Assets"].asString()) )
		return VUWARNING(reader.getLastError().c_str());

	// build asset db
	for ( int iType = 0; iType < tocData.numMembers(); iType++ )
	{
		const std::string &typeName = tocData.getMemberKey(iType);
		const std::string &fileName = tocData[typeName].asString();
		if ( !reader.loadFromFile(mAssetDB[typeName], VuFile::IF()->getRootPath() + fileName) )
			return VUWARNING(reader.getLastError().c_str());
	}

	return true;
}

//*****************************************************************************
bool VuAssetBakery::bakeAsset(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName)
{
	const VuAssetTypeInfo *pTypeInfo = VuAssetFactory::IF()->getAssetTypeInfo(assetType);

	const VuJsonContainer &assetInfo = assetDB()[assetType][assetName];
	if ( !assetInfo.isObject() )
	{
		VUPRINTF("Asset '%s' of type '%s' not found in asset DB.", assetName.c_str(), assetType.c_str());
		return false;
	}
	VUUINT32 assetInfoHash = VuDataUtil::calcHash32(assetInfo);

	// handle platform/sku
	const VuJsonContainer &creationInfo = getCreationInfo(platform, sku, assetInfo);

	// handle languages
	std::vector<std::string> langs;
	creationInfo["Langs"].getMemberKeys(langs);
	langs.push_back("");

	for ( int iLang = 0; iLang < (int)langs.size(); iLang++ )
	{
		const std::string &lang = langs[iLang];
		const VuJsonContainer &info = lang.length() ? creationInfo["Langs"][lang] : creationInfo;

		if ( needToBake(platform, sku, assetType, assetName, lang, assetInfoHash) )
		{
			// bake asset
			VuAssetBakeParams bakeParams(platform, sku, lang);
			if ( !pTypeInfo->mBakeFn(info, bakeParams) )
				return false;

			// automatically add "File" as file dependency, if it exists
			const std::string &fileName = info["File"].asString();
			if ( fileName.length() )
				bakeParams.mDependencies.addFile(fileName);

			// finalize dependencies
			if ( !bakeParams.mDependencies.finalize(platform, sku, lang) )
				return false;

			// save baked data
			if ( !saveBakedFile(platform, sku, assetType, assetName, langs[iLang], assetInfoHash, bakeParams) )
				return false;

			// bake asset dependencies
			for ( int iDep = 0; iDep < bakeParams.mDependencies.assetCount(); iDep++ )
				if ( !bakeAsset(platform, sku, bakeParams.mDependencies.assetEntry(iDep).mType, bakeParams.mDependencies.assetEntry(iDep).mName) )
					return false;
		}
	}

	return true;
}

//*****************************************************************************
bool VuAssetBakery::packAsset(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, VuAssetPackFileWriter &packFileWriter)
{
	const VuJsonContainer &assetInfo = assetDB()[assetType][assetName];
	if ( !assetInfo.isObject() )
	{
		VUPRINTF("Asset '%s' of type '%s' not found in asset DB.", assetName.c_str(), assetType.c_str());
		return false;
	}
	VUUINT32 assetInfoHash = VuDataUtil::calcHash32(assetInfo);

	// handle platform/sku
	const VuJsonContainer &creationInfo = getCreationInfo(platform, sku, assetInfo);

	// handle languages
	std::vector<std::string> langs;
	creationInfo["Langs"].getMemberKeys(langs);
	langs.push_back("");

	for ( int iLang = 0; iLang < (int)langs.size(); iLang++ )
	{
		const std::string &lang = langs[iLang];
//		const VuJsonContainer &info = lang.length() ? creationInfo["Langs"][lang] : creationInfo;

		// load baked data (compressed)
		VuBakedAssetHeader header;
		VuArray<VUBYTE> data(0);
		VuAssetDependencies dependencies;
		if ( !loadBakedFile(platform, sku, assetType, assetName, lang, assetInfoHash, header, dependencies, data, LBFM_COMPRESSED_DATA) )
			return false;

		// add to pack file
		if ( !packFileWriter.write(assetType.c_str(), assetName.c_str(), lang.c_str(), header.mAssetVersion, header.mDataHash, header.mUncompressedDataSize, data, header.mCompressionType) )
			return false;
	}
	return true;
}

//*****************************************************************************
bool VuAssetBakery::packAssetDefinitions(const std::string &platform, const std::string &sku, VuAssetPackFileWriter &packFileWriter)
{
	typedef std::vector<VUUINT32> Languages;
	typedef std::map<std::string, VUUINT32> LanguageMasks;
	Languages languages;
	LanguageMasks languageMasks;
	const VuJsonContainer &devLanguages = VuAssetFactory::IF()->getDevInfo()["Languages"];
	for ( int i = 0; i < devLanguages.size(); i++ )
	{
		languages.push_back(VuHash::fnv32String(devLanguages[i].asCString()));
		languageMasks[devLanguages[i].asString()] = 1<<i;
	}

	// compile asset DB for platform/sku
	typedef std::map<std::string, VuAssetDefinitionEntry> Assets;
	typedef std::map<std::string, Assets> Types;

	Types compiledDB;

	for ( int iType = 0; iType < assetDB().numMembers(); iType++ )
	{
		const std::string &assetType = assetDB().getMemberKey(iType);
		const VuJsonContainer &types = assetDB()[assetType];

		for ( int iAsset = 0; iAsset < types.numMembers(); iAsset++ )
		{
			const std::string &assetName = types.getMemberKey(iAsset);
			const VuJsonContainer &assetData = types[assetName];

			// account for filter
			bool filterResult = true;
			if ( assetData["Filter"].isString() )
			{
				VuFilterExpression filter;
				filter.addVariable("platform", platform.c_str());
				filter.addVariable("sku", sku.c_str());
				if ( !filter.evaluate(assetData["Filter"].asCString()) )
					return VUWARNING(filter.getLastError());
				filterResult = filter.result();
			}

			if ( filterResult)
			{
				// handle platform/sku
				const VuJsonContainer &creationInfo = getCreationInfo(platform, sku, assetData);

				VuAssetDefinitionEntry &entry = compiledDB[assetType][assetName];
				entry.mLanguageOverrides = 0;

				// supported languages
				const VuJsonContainer &languages = creationInfo["Langs"];
				for ( int i = 0; i < languages.numMembers(); i++ )
				{
					const std::string &language = languages.getMemberKey(i);
					LanguageMasks::const_iterator itMask = languageMasks.find(language);
					if ( itMask == languageMasks.end() )
						return VUWARNING("Language '%s' not found in GameConfig.json", language.c_str());
					entry.mLanguageOverrides |= itMask->second;
				}
			}
		}
	}

	// pack asset data
	VuArray<VUBYTE> data;
	VuBinaryDataWriter writer(data);
	writer.configure(platform);

	// write languages
	int languageCount = (int)languages.size();
	writer.writeValue(languageCount);
	for ( int i = 0; i < languageCount; i++ )
		writer.writeValue(languages[i]);

	int typeCount = (int)compiledDB.size();
	writer.writeValue(typeCount);
	for ( Types::const_iterator iter = compiledDB.begin(); iter != compiledDB.end(); iter++ )
	{
		const std::string &assetType = iter->first;
		writer.writeString(assetType);

		const Assets &assets = iter->second;
		int assetCount = (int)assets.size();
		writer.writeValue(assetCount);

		for ( Assets::const_iterator iter = assets.begin(); iter != assets.end(); iter++ )
		{
			const std::string &assetName = iter->first;
			const VuAssetDefinitionEntry &entry = iter->second;

			writer.writeString(assetName);
			writer.writeValue(entry.mLanguageOverrides);
		}
	}

	VUUINT32 hash = VuHash::fnv32(&data.begin(), data.size());

	// compress
	VUUINT32 sizeZLib = VuZLibUtil::calcCompressBound(data.size());
	VuArray<VUBYTE> dataZLib(0);
	dataZLib.resize(sizeZLib);
	VuZLibUtil::compressToMemory(&dataZLib[0], &sizeZLib, &data.begin(), data.size());
	dataZLib.resize(sizeZLib);

	if (!packFileWriter.write("Assets", "AssetData", "", 0, hash, data.size(), dataZLib, VU_ASSET_COMPRESSION_ZLIB))
		return VUWARNING("Failed to write asset data to pack file.");

	return true;
}

//*****************************************************************************
bool VuAssetBakery::getAssetDataHash(const std::string &platform, const std::string &sku, const std::string &language, const std::string &assetType, const std::string &assetName, VUUINT32 &hash)
{
	if ( bakeAsset(platform, sku, assetType, assetName) )
	{
		VUUINT32 assetInfoHash = getAssetInfoHash(assetType, assetName);

		VuBakedAssetHeader header;
		VuArray<VUBYTE> data(0);
		VuAssetDependencies dependencies;
		if ( loadBakedFile(platform, sku, assetType, assetName, language, assetInfoHash, header, dependencies, data, LBFM_HEADER) )
		{
			hash = header.mDataHash;
			return true;
		}
	}

	return false;
}

//*****************************************************************************
bool VuAssetBakery::needToBake(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, VUUINT32 creationInfoHash)
{
	VuBakedAssetHeader header;
	VuArray<VUBYTE> data(0);
	VuAssetDependencies dependencies;

	if ( loadBakedFile(platform, sku, assetType, assetName, lang, creationInfoHash, header, dependencies, data, LBFM_HEADER) )
		if ( dependencies.check(platform, sku, lang) )
			return false;

	return true;
}

//*****************************************************************************
bool VuAssetBakery::loadBakedFile(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, VUUINT32 creationInfoHash, VuBakedAssetHeader &header, VuAssetDependencies &dependencies, VuArray<VUBYTE> &data, eLoadBakedFileMode mode)
{
	const VuAssetTypeInfo *pTypeInfo = VuAssetFactory::IF()->getAssetTypeInfo(assetType);

	bool bSuccess = false;
	
	std::string strBakedFileName;
	getBakedFileName(platform, sku, assetType, assetName, lang, strBakedFileName);

	std::string rootPath = VuFile::IF()->getRootPath();
	if ( !VuFile::IF()->getCachePath().empty() && VuFileHostIO::isHostPath(VuFile::IF()->getRootPath().c_str()) )
		rootPath = VuFile::IF()->getCachePath();

	int fileSize = VuFile::IF()->size(rootPath + strBakedFileName);
	if ( fileSize >= sizeof(VuBakedAssetHeader) )
	{
		// open file
		VUHANDLE hFile = VuFile::IF()->open(rootPath + strBakedFileName, VuFile::MODE_READ);
		if ( hFile )
		{
			// load header
			memset(&header, 0, sizeof(header));
			if ( VuFile::IF()->read(hFile, &header, sizeof(header)) == sizeof(header) )
			{
				// swap endianness if necessary
				if ( header.mMagic == VuEndianUtil::swap(scBakedAssetMagic) )
					header.flipEndianness();

				// make sure everything matches
				if ( header.mMagic == scBakedAssetMagic &&
					 header.mHeaderVersion == scBakedAssetHeaderVersion &&
					 header.mAssetVersion == pTypeInfo->mVersion &&
					 header.mDependencySize + header.mCompressedDataSize + sizeof(header) == fileSize &&
					 header.mCreationInfoHash == creationInfoHash )
				{
					// read dependencies
					VuArray<VUBYTE> dependencyData(0);
					dependencyData.resize(header.mDependencySize);
					if ( VuFile::IF()->read(hFile, &dependencyData.begin(), header.mDependencySize) == header.mDependencySize ) 
					{
						VuBinaryDataReader reader(dependencyData);
						dependencies.deserialize(reader);

						if ( mode == LBFM_HEADER )
						{
							bSuccess = true;
						}
						else if ( mode == LBFM_COMPRESSED_DATA )
						{
							// allocate data
							data.resize(header.mCompressedDataSize);
							if ((VUUINT32)VuFile::IF()->read(hFile, &data.begin(), header.mCompressedDataSize) == header.mCompressedDataSize)
								bSuccess = true;
						}
						else if ( mode == LBFM_UNCOMPRESSED_DATA )
						{
							// allocate data
							data.resize(header.mUncompressedDataSize);

							// load data
							if ( header.mCompressionType == VU_ASSET_COMPRESSION_ZLIB )
							{
								VUUINT32 len = header.mUncompressedDataSize;
								if (VuZLibUtil::uncompressFromFile(hFile, header.mCompressedDataSize, &data.begin(), &len) && (len == header.mUncompressedDataSize))
									bSuccess = true;
							}
							else if ( header.mCompressionType == VU_ASSET_COMPRESSION_LZMA )
							{
								VUUINT32 len = header.mUncompressedDataSize;
								if (VuLzma::uncompressFromFile(hFile, header.mCompressedDataSize, &data.begin(), &len) && (len == header.mUncompressedDataSize))
									bSuccess = true;
							}
							else
							{
								if ((VUUINT32)VuFile::IF()->read(hFile, &data.begin(), header.mCompressedDataSize) == header.mUncompressedDataSize)
									bSuccess = true;
							}

							// check data integrity
							if (header.mDataHash != VuHash::fnv32(&data.begin(), data.size()))
								bSuccess = false;
						}
					}
				}
			}

			// close file
			VuFile::IF()->close(hFile);
		}
	}

	return bSuccess;
}

//*****************************************************************************
bool VuAssetBakery::saveBakedFile(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, VUUINT32 creationInfoHash, const VuAssetBakeParams &bakeParams)
{
	const VuAssetTypeInfo *pTypeInfo = VuAssetFactory::IF()->getAssetTypeInfo(assetType);

	// create dependency data
	VuArray<VUBYTE> dependencyData;
	{
		VuBinaryDataWriter writer(dependencyData);
		writer.configure(platform);

		bakeParams.mDependencies.serialize(writer);
	}

	// determine compression types
	std::set<std::string> compressionTypes;
	if ( pTypeInfo->mCompress && sku != "Editor" )
	{
		const VuJsonContainer &skuInfo = VuAssetFactory::IF()->getDevSkuInfo()[sku];
		if ( skuInfo.hasMember("CompressionTypes") )
		{
			// use compression types specified for sku
			const VuJsonContainer &skuCompressionTypes = skuInfo["CompressionTypes"];
			for ( int i = 0; i < skuCompressionTypes.size(); i++ )
				compressionTypes.insert(skuCompressionTypes[i].asString());
		}
		else
		{
			// default compression types
			compressionTypes.insert("zlib");
			compressionTypes.insert("lzma");
		}
	}

	const VuArray<VUBYTE> &data = bakeParams.mData;

	// determine best method
	eVuAssetCompressionType compressionType = VU_ASSET_COMPRESSION_OFF;
	VuArray<VUBYTE> compressedData(0);
	compressedData.resize(data.size());
	VU_MEMCPY(&compressedData.begin(), compressedData.size(), &data.begin(), data.size());

	if ( compressionTypes.find("zlib") != compressionTypes.end() )
	{
		VUUINT32 sizeZLib = VuZLibUtil::calcCompressBound(data.size());
		VuArray<VUBYTE> dataZLib(0);
		dataZLib.resize(sizeZLib);
		VuZLibUtil::compressToMemory(&dataZLib[0], &sizeZLib, &data.begin(), data.size());
		if ( sizeZLib < (VUUINT32)compressedData.size() )
		{
			compressionType = VU_ASSET_COMPRESSION_ZLIB;
			compressedData.resize(sizeZLib);
			VU_MEMCPY(&compressedData.begin(), compressedData.size(), &dataZLib.begin(), sizeZLib);
		}
	}

	if ( compressionTypes.find("lzma") != compressionTypes.end() )
	{
		VUUINT32 sizeLzma = VuLzma::calcCompressBound(data.size());
		VuArray<VUBYTE> dataLzma(0);
		dataLzma.resize(sizeLzma);
		VuLzma::compressToMemory(&dataLzma[0], &sizeLzma, &data.begin(), data.size());
		if ( sizeLzma < (VUUINT32)compressedData.size() )
		{
			compressionType = VU_ASSET_COMPRESSION_LZMA;
			compressedData.resize(sizeLzma);
			VU_MEMCPY(&compressedData.begin(), compressedData.size(), &dataLzma.begin(), sizeLzma);
		}
	}

	bool bSuccess = true;

	// create header
	VuBakedAssetHeader header;
	header.mMagic = scBakedAssetMagic;
	header.mHeaderVersion = scBakedAssetHeaderVersion;
	header.mAssetVersion = pTypeInfo->mVersion;
	header.mUncompressedDataSize = data.size();
	header.mCompressedDataSize = compressedData.size();
	header.mCreationInfoHash = creationInfoHash;
	header.mDataHash = VuHash::fnv32(&data.begin(), data.size());
	header.mDependencySize = (VUUINT16)dependencyData.size();
	header.mCompressionType = (VUUINT16)compressionType;

	// open file
	std::string strBakedFileName;
	getBakedFileName(platform, sku, assetType, assetName, lang, strBakedFileName);

	std::string path;
	if ( VuFileHostIO::isHostPath(VuFile::IF()->getRootPath().c_str()) )
		path = VuFile::IF()->getCachePath();
	else
		path = VuFile::IF()->getRootPath();

	VuFile::IF()->createDirectory(path + VuFileUtil::getPath(strBakedFileName));
	if ( VUHANDLE hFile = VuFile::IF()->open(path + strBakedFileName, VuFile::MODE_WRITE) )
	{
		// write header
		VuArray<VUBYTE> headerData;
		VuBinaryDataWriter writer(headerData);
		writer.configure(platform);

		writer.writeValue(header.mMagic);
		writer.writeValue(header.mHeaderVersion);
		writer.writeValue(header.mAssetVersion);
		writer.writeValue(header.mUncompressedDataSize);
		writer.writeValue(header.mCompressedDataSize);
		writer.writeValue(header.mCreationInfoHash);
		writer.writeValue(header.mDataHash);
		writer.writeValue(header.mDependencySize);
		writer.writeValue(header.mCompressionType);
		
		bSuccess &= VuFile::IF()->write(hFile, &headerData[0], headerData.size()) == headerData.size();

		// write dependencies
		bSuccess &= VuFile::IF()->write(hFile, &dependencyData[0], dependencyData.size()) == dependencyData.size();

		// write data
		bSuccess &= VuFile::IF()->write(hFile, &compressedData[0], compressedData.size()) == compressedData.size();

		// clean up
		VuFile::IF()->close(hFile);
	}
	else
	{
		VUERROR("Unable to open '%s' for writing.", strBakedFileName.c_str());
	}

	return bSuccess;
}

//*****************************************************************************
bool VuAssetBakery::cacheBakedFile(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang)
{
	if ( VuFileHostIO::isHostPath(VuFile::IF()->getRootPath().c_str()) )
	{
		// copy file
		std::string strBakedFileName;
		getBakedFileName(platform, sku, assetType, assetName, lang, strBakedFileName);

		VuArray<VUBYTE> fileData;
		if ( !VuFileUtil::loadFile(VuFile::IF()->getRootPath() + strBakedFileName, fileData) )
			return false;

		VuFile::IF()->createDirectory(VuFile::IF()->getCachePath() + VuFileUtil::getPath(strBakedFileName));
		if ( !VuFileUtil::saveFile(VuFile::IF()->getCachePath() + strBakedFileName, &fileData[0], fileData.size()) )
			return false;

		// copy asset dependencies
		VuBakedAssetHeader header;
		VU_MEMCPY(&header, sizeof(header), &fileData[0], sizeof(header));
		VuBinaryDataReader reader(&fileData[0] + sizeof(header), header.mDependencySize);
		VuAssetDependencies dependencies;
		dependencies.deserialize(reader);

		for ( int iDep = 0; iDep < dependencies.assetCount(); iDep++ )
			if ( !cacheBakedFile(platform, sku, dependencies.assetEntry(iDep).mType, dependencies.assetEntry(iDep).mName, lang) )
				return false;
	}

	return true;
}

//*****************************************************************************
void VuAssetBakery::getBakedFileName(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, std::string &strBakedFileName)
{
	char hashedAssetName[32];
	VU_SPRINTF(hashedAssetName, sizeof(hashedAssetName), "%0llx", VuHash::fnv64String(assetName.c_str()));

	strBakedFileName = std::string(BAKED_ASSETS_FOLDER) + "/" + platform + "/" + sku + "/" + assetType + "/" + hashedAssetName;

	// handle language
	if ( lang.length() )
	{
		strBakedFileName += '_';
		strBakedFileName += lang;
	}

	VuFileUtil::fixSlashes(strBakedFileName);
}

//*****************************************************************************
bool VuAssetBakery::doesAssetExist(const std::string &assetType, const std::string &assetName)
{
	return assetDB()[assetType].hasMember(assetName);
}

//*****************************************************************************
VUUINT32 VuAssetBakery::getAssetInfoHash(const std::string &assetType, const std::string &assetName)
{
	const VuJsonContainer &assetInfo = assetDB()[assetType][assetName];

	return VuDataUtil::calcHash32(assetInfo);
}

//*****************************************************************************
const VuJsonContainer &VuAssetBakery::getCreationInfo(const std::string &platform, const std::string &sku, const std::string &language, const std::string &assetType, const std::string &assetName)
{
	const VuJsonContainer &assetData = assetDB()[assetType][assetName];

	// handle platform
	const VuJsonContainer &platformCreationInfo = assetData["Plats"].hasMember(platform) ? assetData["Plats"][platform] : assetData;

	// handle sku
	const VuJsonContainer &skuCreationInfo = platformCreationInfo["Skus"].hasMember(sku) ? platformCreationInfo["Skus"][sku] : platformCreationInfo;

	// handle language
	const VuJsonContainer &languageCreationInfo = skuCreationInfo["Langs"].hasMember(sku) ? skuCreationInfo["Langs"][language] : skuCreationInfo;

	return languageCreationInfo;
}

//*****************************************************************************
const VuJsonContainer &VuAssetBakery::getCreationInfo(const std::string &platform, const std::string &sku, const VuJsonContainer &assetData)
{
	// handle platform
	const VuJsonContainer &platformCreationInfo = assetData["Plats"].hasMember(platform) ? assetData["Plats"][platform] : assetData;

	// handle sku
	const VuJsonContainer &skuCreationInfo = platformCreationInfo["Skus"].hasMember(sku) ? platformCreationInfo["Skus"][sku] : platformCreationInfo;

	return skuCreationInfo;
}

//*****************************************************************************
void VuAssetBakery::editorSetAssetData(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData)
{
	mAssetDB[strType][strAsset] = assetData;
}

//*****************************************************************************
void VuAssetBakery::editorCreateAsset(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData)
{
	mAssetDB[strType][strAsset] = assetData;
}

//*****************************************************************************
void VuAssetBakery::editorDeleteAsset(const std::string &strType, const std::string &strAsset)
{
	mAssetDB[strType].removeMember(strAsset);
}

//*****************************************************************************
void VuBakedAssetHeader::flipEndianness()
{
	VuEndianUtil::swapInPlace(mMagic);
	VuEndianUtil::swapInPlace(mHeaderVersion);
	VuEndianUtil::swapInPlace(mAssetVersion);
	VuEndianUtil::swapInPlace(mUncompressedDataSize);
	VuEndianUtil::swapInPlace(mCompressedDataSize);
	VuEndianUtil::swapInPlace(mCreationInfoHash);
	VuEndianUtil::swapInPlace(mDataHash);
	VuEndianUtil::swapInPlace(mDependencySize);
	VuEndianUtil::swapInPlace(mCompressionType);
}
