//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Factory class
// 
//*****************************************************************************

#include "VuAssetFactory.h"
#include "VuAssetPackFile.h"
#include "VuAssetBakery.h"
#include "VuAsset.h"
#include "VuEngine/VuEngineRegistry.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFilterExpression.h"
#include "VuEngine/Util/VuZLibUtil.h"
#include "VuEngine/Util/VuLzma.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Dev/VuDevHostComm.h"



// internal data
class VuAssetEntry
{
public:
	VuAssetEntry() : mInfoHash(0), mLanguageOverrides(0), mUsed(false) {}

	VUUINT32	mInfoHash; // 0 when loading from pack files
	VUUINT32	mLanguageOverrides;
	bool		mUsed;
};

class VuAssetDB
{
public:
	VuAssetDB() : mVersion(-1) {}

	typedef std::hash_map<VUUINT32, VuAssetEntry> Assets;
	typedef std::vector<std::string> AssetNames;
	typedef std::map<std::string, AssetNames> AssetTypes;
	typedef std::vector<VUUINT32> Languages;

	bool			loadRaw(const std::string &strType, const VuJsonContainer &info);
	bool			loadPacked(const VuArray<VUBYTE> &data);
	VuAssetEntry	*getAssetEntry(const std::string &strType, const std::string &strAsset);
	VUUINT32		getLanguageMask(const std::string &language);

	std::string				mSku;
	int						mVersion;
	Assets					mAssets;
	AssetTypes				mAssetTypes;
	Languages				mLanguages;
	VuAssetPackFileReader	mPackFile;
	VuAssetPackFileReader	mExpansionFile;
};

class VuAssetFactory::CreateAssetStackPusher
{
public:
	CreateAssetStackPusher(VuAsset *pAsset)
	{
		VuAssetFactory::IF()->mCreateAssetStack.push(pAsset);
		pAsset->mChildAssets.clear();
	}
	~CreateAssetStackPusher()
	{
		VuAssetFactory::IF()->mCreateAssetStack.pop();
	}
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAssetFactory, VuAssetFactory);


//*****************************************************************************
VuAssetFactory::VuAssetFactory():
	mpAssetDB(VUNULL),
	mAssetsLoadedCount(0)
{
}

//*****************************************************************************
bool VuAssetFactory::init(void (*pRegisterGameAssetsFn)(), const std::string &sku)
{
	mSku = sku;

	mLanguage = VuSys::IF()->getLanguage();
	if ( VuDevConfig::IF() )
		VuDevConfig::IF()->getParam("Language").getValue(mLanguage);

	// dev mode?
	if ( VuFile::IF()->exists(VuFile::IF()->getRootPath() + "GameInfo.json") )
	{
		VuJsonReader reader;
		if ( !reader.loadFromFile(mDevGameInfo, VuFile::IF()->getRootPath() + "GameInfo.json") )
			return VUWARNING("Unable to load 'GameInfo.json': %s", reader.getLastError().c_str());
	}

	// register engine asset types
	VuEngineRegistry::addAssetTypes();

	// register game asset types
	if ( pRegisterGameAssetsFn )
		pRegisterGameAssetsFn();

	// load asset DB
	if ( !loadAssetDB() )
		return false;

	// use sku from base asset DB
	mSku = mpAssetDB->mSku;

	if ( VuDev::IF() )
	{
		VuDev::IF()->setBuildNumber(mpAssetDB->mVersion);
	}

	// set up asset type priorities
	mAssetTypePriorities["VuAnimatedModelAsset"] = 1;
	mAssetTypePriorities["VuStaticModelAsset"] = 1;
	mAssetTypePriorities["VuPfxAsset"] = 2;

	return true;
}

//*****************************************************************************
void VuAssetFactory::postInit()
{
}

//*****************************************************************************
void VuAssetFactory::preRelease()
{
	for ( auto &iter : mPreloadedAssets )
		releaseAsset(iter.second);
	mPreloadedAssets.clear();

	clearAssetCache();
}

//*****************************************************************************
void VuAssetFactory::release()
{
	if ( mRepository.size() )
	{
		VUPRINTF("Dangling Assets:\n");
		for ( const auto &iter : mRepository )
			VUPRINTF("%s %s\n", iter.second->getType(), iter.second->getAssetName().c_str());
		VUASSERT(0, "VuAssetFactory::release() dangling assets");
	}

	unloadAssetDB();

	mTypeNames.clear();
	mTypes.clear();
	mRepository.clear();
}

//*****************************************************************************
void VuAssetFactory::registerType(const std::string &strType, AssetCreateFn createFn, AssetSchemaFn schemaFn, AssetBakeFn bakeFn, VUUINT32 version, bool compress)
{
	mTypeNames.push_back(strType);
	std::sort(mTypeNames.begin(), mTypeNames.end());

	VuAssetTypeInfo &typeInfo = mTypes[strType];
	typeInfo.mCreateFn = createFn;
	typeInfo.mSchemaFn = schemaFn;
	typeInfo.mBakeFn = bakeFn;
	typeInfo.mVersion = version;
	typeInfo.mCompress = compress;
}

//*****************************************************************************
VuAsset *VuAssetFactory::createAsset(const std::string &strType, const std::string &strAsset, int flags)
{
	VUUINT32 hashID = calcAssetHashID(strType.c_str(), strAsset.c_str());
	if ( VuAsset *pAsset = findAsset(hashID) )
	{
		pAsset->addRef();

		// listeners
		for ( auto &listener : mListeners )
			listener->onCreateAsset(pAsset);

		if ( mCreateAssetStack.size() )
			mCreateAssetStack.top()->mChildAssets.push_back(pAsset->getHashID());

		return pAsset;
	}

	// find first asset entry
	VuAssetEntry *pAssetEntry = mpAssetDB->getAssetEntry(strType, strAsset);
	if ( pAssetEntry == VUNULL )
	{
		if ( !(flags & OPTIONAL_ASSET) )
			VUERROR("Asset %s of type %s does not exist!", strAsset.c_str(), strType.c_str());
		return VUNULL;
	}
	pAssetEntry->mUsed = true;

	const VuAssetTypeInfo *pTypeInfo = getAssetTypeInfo(strType);
	if ( pTypeInfo == VUNULL )
	{
		VUERROR("Asset type %s does not exist!", strType.c_str());
		return VUNULL;
	}

	if ( pTypeInfo->mCreateFn == VUNULL )
	{
		VUERROR("Asset type %s not registered!", strType.c_str());
		return VUNULL;
	}

	// we're going to load an asset
	mAssetsLoadedCount++;
	VuGfxSort::IF()->flush();

	// create asset
	VuAsset *pAsset = pTypeInfo->mCreateFn();
	pAsset->setAssetName(strAsset, hashID);

	// load
	if ( !loadAsset(pTypeInfo, pAssetEntry, pAsset) )
	{
		VUWARNING("Error loading %s asset '%s'!", strType.c_str(), strAsset.c_str());
	}

	// add to repository
	mRepository[hashID] = pAsset;

	// listeners
	for ( auto &listener : mListeners )
		listener->onCreateAsset(pAsset);

	if ( mCreateAssetStack.size() )
		mCreateAssetStack.top()->mChildAssets.push_back(pAsset->getHashID());

	return pAsset;
}

//*****************************************************************************
void VuAssetFactory::addAssetRef(VuAsset *pAsset)
{
	VUASSERT(pAsset, "VuAssetFactory::addAssetRef() NULL asset.");

	pAsset->addRef();
}

//*****************************************************************************
void VuAssetFactory::releaseAsset(VuAsset *pAsset)
{
	if ( pAsset )
	{
		// we may be letting go of assets used by the render thread
		VuGfxSort::IF()->flush();

		pAsset->removeRef();
		if ( pAsset->refCount() == 0 )
		{
			// remove from repository
			Repository::iterator itAsset = mRepository.find(pAsset->getHashID());
			if ( itAsset != mRepository.end() )
				mRepository.erase(itAsset);

			delete pAsset;
		}
	}
}

//*****************************************************************************
bool VuAssetFactory::doesAssetExist(const std::string &strType, const std::string &strAsset)
{
	if ( mpAssetDB->getAssetEntry(strType, strAsset) )
		return true;

	return false;
}

//*****************************************************************************
const VuAssetFactory::AssetTypes &VuAssetFactory::getAssetTypes()
{
	return mTypeNames;
}

//*****************************************************************************
const VuAssetFactory::AssetNames &VuAssetFactory::getAssetNames(const std::string &strType)
{
	VuAssetDB::AssetTypes::const_iterator iter = mpAssetDB->mAssetTypes.find(strType);
	if ( iter != mpAssetDB->mAssetTypes.end() )
		return iter->second;

	static AssetNames sNoNames;
	return sNoNames;
}

//*****************************************************************************
VUUINT32 VuAssetFactory::getAssetInfoHash(const std::string &strType, const std::string &strAsset)
{
	if ( const VuAssetEntry *pAssetEntry = mpAssetDB->getAssetEntry(strType, strAsset) )
		return pAssetEntry->mInfoHash;

	return 0;
}

//*****************************************************************************
void VuAssetFactory::forgetAsset(const std::string &strType, const std::string &strAsset)
{
	// in order to 'forget' an asset, simply let go of our weak reference

	// remove from repository
	Repository::iterator itAsset = mRepository.find(calcAssetHashID(strType.c_str(), strAsset.c_str()));
	if ( itAsset != mRepository.end() )
		mRepository.erase(itAsset);
}

//*****************************************************************************
bool VuAssetFactory::reloadAssetInfo()
{
	delete mpAssetDB;
	mpAssetDB = VUNULL;

	mRepository.clear();

	if ( !loadAssetDB() )
		return false;

	return true;
}

//*****************************************************************************
void VuAssetFactory::reloadAsset(VuAsset *pAsset)
{
	VuGfxSort::IF()->flush();

	pAsset->unload();

	const std::string assetType = pAsset->getType();
	const std::string &assetName = pAsset->getAssetName().c_str();

	// load
	const VuAssetTypeInfo *pTypeInfo = getAssetTypeInfo(assetType);
	VuAssetEntry *pAssetEntry = mpAssetDB->getAssetEntry(assetType, assetName);

	if ( !loadAsset(pTypeInfo, pAssetEntry, pAsset) )
		VUWARNING("Error reloading %s asset '%s'!", assetType.c_str(), assetName.c_str());
}

//*****************************************************************************
VuAsset *VuAssetFactory::findAsset(const char *strType, const char *strAsset)
{
	VUUINT32 hashID = calcAssetHashID(strType, strAsset);
	return findAsset(hashID);
}

//*****************************************************************************
VuAsset *VuAssetFactory::findAsset(VUUINT32 assetHash)
{
	Repository::iterator itAsset = mRepository.find(assetHash);
	if ( itAsset != mRepository.end() )
		return itAsset->second;

	return VUNULL;
}

//*****************************************************************************
bool VuAssetFactory::wasAssetUsed(const std::string &strType, const std::string &strAsset)
{
	if ( VuAssetEntry *pAssetEntry = mpAssetDB->getAssetEntry(strType, strAsset) )
		return pAssetEntry->mUsed;

	return false;
}

//*****************************************************************************
int VuAssetFactory::getAssetTypePriority(const std::string &strType)
{
	AssetTypePriorities::const_iterator iter = mAssetTypePriorities.find(strType);
	if ( iter == mAssetTypePriorities.end() )
		return 0;

	return iter->second;
}

//*****************************************************************************
void VuAssetFactory::editorSetAssetData(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData)
{
	VUUINT32 hashID = calcAssetHashID(strType.c_str(), strAsset.c_str());
	VuAssetEntry &assetEntry = mpAssetDB->mAssets[hashID];

	// info hash
	assetEntry.mInfoHash = VuDataUtil::calcHash32(assetData);

	// supported languages
	assetEntry.mLanguageOverrides = 0;
	const VuJsonContainer &creationInfo = VuAssetBakery::getCreationInfo(VUPLATFORM, mSku, assetData);
	const VuJsonContainer &languages = creationInfo["Langs"];
	for ( int i = 0; i < languages.numMembers(); i++ )
		assetEntry.mLanguageOverrides |= mpAssetDB->getLanguageMask(languages.getMemberKey(i));

	if ( VuAsset *pAsset = VuAssetFactory::IF()->findAsset(strType.c_str(), strAsset.c_str()) )
	{
		pAsset->editorReload();
	}
}

//*****************************************************************************
void VuAssetFactory::editorCreateAsset(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData)
{
	VUUINT32 hashID = calcAssetHashID(strType.c_str(), strAsset.c_str());
	VuAssetEntry &assetEntry = mpAssetDB->mAssets[hashID];

	// info hash
	assetEntry.mInfoHash = VuDataUtil::calcHash32(assetData);

	// supported languages
	assetEntry.mLanguageOverrides = 0;
	const VuJsonContainer &creationInfo = VuAssetBakery::getCreationInfo(VUPLATFORM, mSku, assetData);
	const VuJsonContainer &languages = creationInfo["Langs"];
	for ( int i = 0; i < languages.numMembers(); i++ )
		assetEntry.mLanguageOverrides |= mpAssetDB->getLanguageMask(languages.getMemberKey(i));

	// update type info
	AssetNames &assetNames = mpAssetDB->mAssetTypes[strType];
	assetNames.push_back(strAsset);
	std::sort(assetNames.begin(), assetNames.end());
	assetNames.erase(std::unique(assetNames.begin(), assetNames.end()), assetNames.end());
}

//*****************************************************************************
void VuAssetFactory::editorDeleteAsset(const std::string &strType, const std::string &strAsset)
{
	VUUINT32 hashID = calcAssetHashID(strType.c_str(), strAsset.c_str());

	// creation info
	VuAssetDB::Assets::iterator itAsset = mpAssetDB->mAssets.find(hashID);
	if ( itAsset != mpAssetDB->mAssets.end() )
		mpAssetDB->mAssets.erase(itAsset);

	// type info
	AssetNames &assetNames = mpAssetDB->mAssetTypes[strType];
	AssetNames::iterator itAssetName = std::find(assetNames.begin(), assetNames.end(), strAsset);
	if ( itAssetName != assetNames.end() )
		assetNames.erase(itAssetName);
}

//*****************************************************************************
const VuAssetTypeInfo *VuAssetFactory::getAssetTypeInfo(const std::string &strType)
{
	Types::iterator iter = mTypes.find(strType);
	return iter == mTypes.end() ? VUNULL : &iter->second;
}

//*****************************************************************************
bool VuAssetFactory::isPackFileOpen()
{
	return mpAssetDB && mpAssetDB->mPackFile.handle();
}

//*****************************************************************************
bool VuAssetFactory::openExpansionFile()
{
	return mpAssetDB->mExpansionFile.open(VuFile::IF()->getRootPath() + "Expansion.apf");
}

//*****************************************************************************
void VuAssetFactory::preloadAsset(const std::string &assetType, const std::string &assetName)
{
	VUUINT32 hashID = calcAssetHashID(assetType.c_str(), assetName.c_str());

	// alread preloaded?
	Repository::iterator itAsset = mPreloadedAssets.find(hashID);
	if ( itAsset == mPreloadedAssets.end() )
	{
		if ( VuAsset *pAsset = createAsset(assetType, assetName, VuAssetFactory::OPTIONAL_ASSET) )
			mPreloadedAssets[hashID] = pAsset;
		else
			VUPRINTF("WARNING:  Preload asset %s of type %s not found!\n", assetName.c_str(), assetType.c_str());
	}
}

//*****************************************************************************
void VuAssetFactory::cacheAsset(VuAsset *pAsset)
{
	addAssetRef(pAsset);
	mCachedAssets.push_back(pAsset);
}

//*****************************************************************************
void VuAssetFactory::clearAssetCache()
{
	for ( auto &pAsset : mCachedAssets )
		releaseAsset(pAsset);
	mCachedAssets.clear();
}

//*****************************************************************************
bool VuAssetFactory::loadAssetDB()
{
	// already loaded?
	if ( mpAssetDB )
		return true;

	bool bSuccess = true;
	if ( VuFile::IF()->exists(VuFile::IF()->getRootPath() + "Assets.apf") )
		bSuccess = loadPackedAssetDB();
	else
		bSuccess = loadRawAssetDB();
	if ( !bSuccess )
		return VUWARNING("Unable to load asset DB.");

	return true;
}

//*****************************************************************************
void VuAssetFactory::unloadAssetDB()
{
	delete mpAssetDB;
	mpAssetDB = VUNULL;
}

//*****************************************************************************
bool VuAssetFactory::loadRawAssetDB()
{
	// find package
	VuAssetDB *pAssetDB = new VuAssetDB;

	// load TOC
	VuJsonContainer tocData;
	VuJsonReader reader;
	if ( !reader.loadFromFile(tocData, VuFile::IF()->getRootPath() + getDevPackageInfo()["Assets"].asString()) )
	{
		delete pAssetDB;
		return VUWARNING(reader.getLastError().c_str());
	}

	// determine content category
	pAssetDB->mSku = mSku;
	pAssetDB->mVersion = getDevPackageInfo()["Version"].asInt();

	for ( Types::iterator iter = mTypes.begin(); iter != mTypes.end(); iter++ )
	{
		const std::string &typeName = iter->first;
		if ( tocData.hasMember(typeName) )
		{
			const std::string &fileName = tocData[typeName].asString();

			VuJsonContainer assetData;
			if ( !reader.loadFromFile(assetData, VuFile::IF()->getRootPath() + fileName) )
			{
				delete pAssetDB;
				return VUWARNING(reader.getLastError().c_str());
			}

			if ( !pAssetDB->loadRaw(typeName, assetData) )
			{
				delete pAssetDB;
				return VUWARNING("Unable to load asset info for '%s'.", typeName.c_str());
			}
		}
	}

	mpAssetDB = pAssetDB;

	return true;
}

//*****************************************************************************
bool VuAssetFactory::loadPackedAssetDB()
{
	VuAssetDB *pAssetDB = new VuAssetDB;

	// open pack file
	if ( !pAssetDB->mPackFile.open(VuFile::IF()->getRootPath() + "Assets.apf") )
	{
		delete pAssetDB;
		return false;
	}

	// determine content category
	pAssetDB->mSku = pAssetDB->mPackFile.getSku();
	pAssetDB->mVersion = pAssetDB->mPackFile.getVersion();

	// load asset data
	VuAssetPackFileReader::eSeekResult result;
	VuAssetPackFileReader::Entry entry;
	result = pAssetDB->mPackFile.seek("Assets", "AssetData", "", entry);

	if ( result == VuAssetPackFileReader::RESULT_SUCCESS )
	{
		// uncompress
		VuArray<VUBYTE> data;
		data.resize(entry.mUncompressedSize);
		VUUINT32 len = entry.mUncompressedSize;
		if (!VuZLibUtil::uncompressFromFile(pAssetDB->mPackFile.handle(), entry.mCompressedSize, &data.begin(), &len) || len != entry.mUncompressedSize)
		{
			delete pAssetDB;
			return VUWARNING("Unable to uncompress asset data");
		}

		if ( !pAssetDB->loadPacked(data) )
			return VUWARNING("Unable to load asset info.");
	}
	else if ( result == VuAssetPackFileReader::RESULT_ERROR )
	{
		delete pAssetDB;
		return false;
	}

	mpAssetDB = pAssetDB;

	return true;
}

//*****************************************************************************
bool VuAssetFactory::loadAsset(const VuAssetTypeInfo *pTypeInfo, const VuAssetEntry *pAssetEntry, VuAsset *pAsset)
{
	CreateAssetStackPusher stackPusher(pAsset);

	std::string assetType = pAsset->getType();
	const std::string &assetName = pAsset->getAssetName();
	std::string language;
	if ( pAssetEntry->mLanguageOverrides & mpAssetDB->getLanguageMask(mLanguage) )
		language = mLanguage;

	// try to load from packfile
	if ( VUHANDLE packFileHandle = mpAssetDB->mPackFile.handle() )
	{
		VuAssetPackFileReader::eSeekResult result;
		VuAssetPackFileReader::Entry entry;
		result = mpAssetDB->mPackFile.seek(assetType, assetName, language, entry);
		if ( result != VuAssetPackFileReader::RESULT_SUCCESS )
		{
			// try expansion file
			packFileHandle = mpAssetDB->mExpansionFile.handle();
			if ( packFileHandle )
				result = mpAssetDB->mExpansionFile.seek(assetType, assetName, language, entry);
		}

		if ( result != VuAssetPackFileReader::RESULT_SUCCESS )
			return false;

		// verify version
		if ( entry.mVersion != pTypeInfo->mVersion )
			return VUWARNING("Version mismatch in packfile data: %s.%s.%s", assetType.c_str(), assetName.c_str(), language.c_str());

		// allocate data
		VuArray<VUBYTE> data;
		data.resize(entry.mUncompressedSize);

		// load data
		if (entry.mCompressionType == VU_ASSET_COMPRESSION_ZLIB)
		{
			VUUINT32 len = entry.mUncompressedSize;
			if (!VuZLibUtil::uncompressFromFile(packFileHandle, entry.mCompressedSize, &data.begin(), &len) || len != entry.mUncompressedSize)
				return VUWARNING("Unable to uncompress packfile data: %s.%s.%s", assetType.c_str(), assetName.c_str(), language.c_str());
		}
		else if (entry.mCompressionType == VU_ASSET_COMPRESSION_LZMA)
		{
			VUUINT32 len = entry.mUncompressedSize;
			if (!VuLzma::uncompressFromFile(packFileHandle, entry.mCompressedSize, &data.begin(), &len) || len != entry.mUncompressedSize)
				return VUWARNING("Unable to uncompress packfile data: %s.%s.%s", assetType.c_str(), assetName.c_str(), language.c_str());
		}
		else
		{
			if ((VUUINT32)VuFile::IF()->read(packFileHandle, &data.begin(), entry.mCompressedSize) != entry.mUncompressedSize)
				return VUWARNING("Unable to read packfile data: %s.%s.%s", assetType.c_str(), assetName.c_str(), language.c_str());
		}

		// check data integrity
		#if VERIFY_PACKED_ASSET_HASH
			if (entry.mHash != VuHash::fnv32(&data.begin(), data.size()))
				return VUWARNING("Packfile integrity failure: %s.%s.%s", assetType.c_str(), assetName.c_str(), language.c_str());
		#else
			VUASSERT(entry.mHash == VuHash::fnv32(&data.begin(), data.size()), "VuAssetPackFileReader integrity failure");
		#endif

		// load from baked data
		VuBinaryDataReader reader(data);
		if ( !pAsset->load(reader) )
			return VUWARNING("Unable to load from packfile data: %s.%s.%s", assetType.c_str(), assetName.c_str(), language.c_str());

		return true;
	}

	VUUINT32 assetInfoHash = getAssetInfoHash(assetType, assetName);

	// load baked data
	{
		VuBakedAssetHeader header;
		VuArray<VUBYTE> data(0);
		VuAssetDependencies dependencies;
		if ( VuAssetBakery::loadBakedFile(VUPLATFORM, mSku, assetType, assetName, language, assetInfoHash, header, dependencies, data, VuAssetBakery::LBFM_UNCOMPRESSED_DATA) )
		{
			if ( dependencies.check(language) )
			{
				VuBinaryDataReader reader(data);
				if ( pAsset->load(reader) )
				{
					return true;
				}
			}
		}
	}

	if ( VuDevHostComm::IF() )
	{
		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("asset");
		request.writeString("bake");
		request.writeString(VUPLATFORM);
		request.writeString(mSku);
		request.writeString(assetType);
		request.writeString(assetName);

		if ( !VuDevHostComm::IF()->sendMessage(true) )
			return false;

		VuBinaryDataReader response = VuDevHostComm::IF()->response();
		const char *success = response.readString();
		if ( strcmp(success, "true") != 0 )
			return false;

		// copy baked file (and dependencies) to cache folder
		if ( !VuFile::IF()->getCachePath().empty() )
			if ( !VuAssetBakery::cacheBakedFile(VUPLATFORM, mSku, assetType, assetName, language) )
				return false;

		VuBakedAssetHeader header;
		VuArray<VUBYTE> data(0);
		VuAssetDependencies dependencies;
		if ( !VuAssetBakery::loadBakedFile(VUPLATFORM, mSku, assetType, assetName, language, assetInfoHash, header, dependencies, data, VuAssetBakery::LBFM_UNCOMPRESSED_DATA) )
			return false;

		VuBinaryDataReader reader(data);
		if ( !pAsset->load(reader) )
			return false;
	}
	else if ( VuAssetBakery::IF() )
	{
		VUPRINTF("Baking %s.%s.%s\n", assetType.c_str(), assetName.c_str(), language.c_str());

		const VuJsonContainer &creationInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, mSku, language, assetType, assetName);

		// bake asset
		VuAssetBakeParams bakeParams(VUPLATFORM, mSku, language);
		if ( !getAssetTypeInfo(assetType)->mBakeFn(creationInfo, bakeParams) )
			return false;

		// load asset
		VuBinaryDataReader reader(bakeParams.mData);
		if ( !pAsset->load(reader) )
			return false;

		// automatically add "File" as file dependency, if it exists
		const std::string &fileName = creationInfo["File"].asString();
		if ( fileName.length() )
			bakeParams.mDependencies.addFile(fileName);

		// finalize dependencies
		if ( !bakeParams.mDependencies.finalize(VUPLATFORM, mSku, language) )
			return false;

		// save baked data
		if ( !VuAssetBakery::saveBakedFile(VUPLATFORM, mSku, assetType, assetName, language, assetInfoHash, bakeParams) )
			return false;
	}
	else
	{
		return false;
	}

	return true;
}

//*****************************************************************************
bool VuAssetDB::loadRaw(const std::string &strType, const VuJsonContainer &info)
{
	const VuJsonContainer &languages = VuAssetFactory::IF()->getDevInfo()["Languages"];
	for ( int i = 0; i < languages.size(); i++ )
		mLanguages.push_back(VuHash::fnv32String(languages[i].asCString()));

	AssetNames &assetNames = mAssetTypes[strType];

	for ( int i = 0; i < info.numMembers(); i++ )
	{
		const std::string &assetName = info.getMemberKey(i);
		const VuJsonContainer &assetData = info[assetName];

		// account for filter
		bool filterResult = true;
		if ( assetData["Filter"].isString() && mSku != "Editor" )
		{
			VuFilterExpression filter;
			filter.addVariable("sku", mSku.c_str());
			filter.addVariable("platform", VUPLATFORM);
			if ( !filter.evaluate(assetData["Filter"].asCString()) )
				return VUWARNING(filter.getLastError());
			filterResult = filter.result();
		}

		if ( filterResult)
		{
			// add asset
			VUUINT32 hashID = VuAssetFactory::calcAssetHashID(strType.c_str(), assetName.c_str());

			// check for name collision
			if ( mAssets.find(hashID) != mAssets.end() )
			{
				VUPRINTF("Asset collision!\n");
				return false;
			}

			// info hash
			VuAssetEntry &assetEntry = mAssets[hashID];
			assetEntry.mInfoHash = VuDataUtil::calcHash32(assetData);

			// supported languages
			const VuJsonContainer &creationInfo = VuAssetBakery::getCreationInfo(VUPLATFORM, mSku, assetData);
			const VuJsonContainer &languages = creationInfo["Langs"];
			for ( int i = 0; i < languages.numMembers(); i++ )
				assetEntry.mLanguageOverrides |= getLanguageMask(languages.getMemberKey(i));

			// add to asset names
			assetNames.push_back(assetName);
			std::sort(assetNames.begin(), assetNames.end());
		}
	}

	return true;
}

//*****************************************************************************
bool VuAssetDB::loadPacked(const VuArray<VUBYTE> &data)
{
	VuBinaryDataReader reader(data);
	VuJsonBinaryReader jsonReader;

	// read languages
	int languageCount;
	reader.readValue(languageCount);
	mLanguages.resize(languageCount);
	for ( int i = 0; i < languageCount; i++ )
		reader.readValue(mLanguages[i]);

	// read asset names
	int typeCount;
	reader.readValue(typeCount);
	for ( int i = 0; i < typeCount; i++ )
	{
		const char *assetType = reader.readString();
		AssetNames &assetNames = mAssetTypes[assetType];

		VUUINT32 hashedType = VuHash::fnv32String(assetType); // calcAssetHashID optimization

		int assetCount;
		reader.readValue(assetCount);

		assetNames.resize(assetCount);
		for	( int j = 0; j < assetCount; j++ )
		{
			const char *assetName = reader.readString();
			assetNames[j] = assetName;

			VUUINT32 hashID = VuHash::fnv32String(assetName, hashedType); // calcAssetHashID optimization

			VuAssetEntry &assetEntry = mAssets[hashID];
			reader.readValue(assetEntry.mLanguageOverrides);
		}
	}

	return true;
}

//*****************************************************************************
VuAssetEntry *VuAssetDB::getAssetEntry(const std::string &strType, const std::string &strAsset)
{
	VUUINT32 hashID = VuAssetFactory::calcAssetHashID(strType.c_str(), strAsset.c_str());

	Assets::iterator itAssets = mAssets.find(hashID);
	if ( itAssets != mAssets.end() )
		return &itAssets->second;

	return VUNULL;
}

//*****************************************************************************
VUUINT32 VuAssetDB::getLanguageMask(const std::string &language)
{
	VUUINT32 hash = VuHash::fnv32String(language.c_str());

	int code = 0;
	for ( Languages::const_iterator iter = mLanguages.begin(); iter != mLanguages.end(); iter++ )
	{
		if ( *iter == hash )
			return (1<<code);
		code++;
	}

	return 0;
}
