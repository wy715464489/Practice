//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Factory class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Json/VuJsonContainer.h"

class VuEngine;
class VuAsset;
class VuJsonContainer;
class VuAssetBakeParams;
class VuAssetTypeInfo;
class VuAssetEntry;
class VuAssetDB;

class VuAssetFactory : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuAssetFactory)

public:
	VuAssetFactory();

	virtual bool	init(void (*pRegisterGameAssetsFn)(), const std::string &sku);
	virtual void	postInit();
	virtual void	preRelease();
	virtual void	release();

	typedef std::vector<std::string> AssetTypes;
	typedef std::vector<std::string> AssetNames;
	typedef std::hash_map<VUUINT32, VuAsset *> Repository;

	typedef VuAsset *(*AssetCreateFn)();
	typedef void (*AssetSchemaFn)(const VuJsonContainer &, VuJsonContainer &);
	typedef bool (*AssetBakeFn)(const VuJsonContainer &, VuAssetBakeParams &);

	// register an asset type
	void				registerType(const std::string &strType, AssetCreateFn createFn, AssetSchemaFn schemaFn, AssetBakeFn bakeFn, VUUINT32 version, bool compress);
	
	// create/release
	enum { OPTIONAL_ASSET = 0x1 };
	template<class T>
	T					*createAsset(const std::string &strAsset, int flags = 0);
	VuAsset				*createAsset(const std::string &strType, const std::string &strAsset, int flags = 0);
	void				addAssetRef(VuAsset *pAsset);
	void				releaseAsset(VuAsset *pAsset);

	// enumeration
	template<class T>
	bool				doesAssetExist(const std::string &strAsset);
	bool				doesAssetExist(const std::string &strType, const std::string &strAsset);
	const AssetTypes	&getAssetTypes();
	template<class T>
	const AssetNames	&getAssetNames();
	const AssetNames	&getAssetNames(const std::string &strType);
	VUUINT32			getAssetInfoHash(const std::string &strType, const std::string &strAsset);

	// force reloading of an asset (ref counting handles issues with old assets)
	template<class T>
	void				forgetAsset(const std::string &strAsset);
	void				forgetAsset(const std::string &strType, const std::string &strAsset);

	// reloads the asset info
	bool				reloadAssetInfo();

	// reloads the asset from disk
	void				reloadAsset(VuAsset *pAsset);

	// repository
	VuAsset				*findAsset(const char *strType, const char *strAsset);
	VuAsset				*findAsset(VUUINT32 assetHash);
	bool				wasAssetUsed(const std::string &strType, const std::string &strAsset);
	const Repository	&getRepository() { return mRepository; }

	// sku
	const std::string	&getSku() { return mSku; }

	// asset type priority
	int					getAssetTypePriority(const std::string &strType);

	// dev package info
	const VuJsonContainer	&getDevInfo() const			{ return mDevGameInfo; }
	const VuJsonContainer	&getDevPackageInfo() const	{ return mDevGameInfo["Package"]; }
	const VuJsonContainer	&getDevSkuInfo() const		{ return mDevGameInfo["Skus"]; }

	// manipulation
	void					editorSetAssetData(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData);
	void					editorCreateAsset(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData);
	void					editorDeleteAsset(const std::string &strType, const std::string &strAsset);

	// type info
	const VuAssetTypeInfo	*getAssetTypeInfo(const std::string &strType);

	// stats
	int					getAssetsLoadedCount() { return mAssetsLoadedCount; }

	// expansion file
	bool				isPackFileOpen();
	bool				openExpansionFile();

	// preloading/caching
	void				preloadAsset(const std::string &assetType, const std::string &assetName);
	void				cacheAsset(VuAsset *pAsset);
	void				clearAssetCache();

	// listeners
	class VuListener
	{
	public:
		virtual void	onCreateAsset(VuAsset *pAsset) {}
	};
	void			addListener(VuListener *pListener)		{ mListeners.push_back(pListener); }
	void			removeListener(VuListener *pListener)	{ mListeners.remove(pListener); }

	// hash
	static inline VUUINT32	calcAssetHashID(const char *strType, const char *strAsset);

private:
	typedef std::map<std::string, VuAssetTypeInfo> Types;
	typedef std::vector<std::string> TypeNames;
	typedef std::map<std::string, int> AssetTypePriorities;
	typedef std::list<VuAsset *> CachedAssets;
	typedef std::list<VuListener *> Listeners;
	typedef std::stack<VuAsset *> CreateAssetStack;
	class CreateAssetStackPusher;

	bool				loadAssetDB();
	void				unloadAssetDB();
	bool				loadRawAssetDB();
	bool				loadPackedAssetDB();
	bool				loadAsset(const VuAssetTypeInfo *pTypeInfo, const VuAssetEntry *pAssetEntry, VuAsset *pAsset);

	std::string			mSku;
	std::string			mLanguage;
	AssetTypes			mTypeNames;
	Types				mTypes;
	VuAssetDB			*mpAssetDB;
	Repository			mRepository;
	VuJsonContainer		mDevGameInfo;
	AssetTypePriorities	mAssetTypePriorities;
	int					mAssetsLoadedCount;
	Repository			mPreloadedAssets;
	CachedAssets		mCachedAssets;
	Listeners			mListeners;
	CreateAssetStack	mCreateAssetStack;

};

class VuAssetTypeInfo
{
public:
	VuAssetTypeInfo() : mCreateFn(VUNULL), mSchemaFn(VUNULL), mBakeFn(VUNULL), mVersion(0), mCompress(false) {}

	VuAssetFactory::AssetCreateFn	mCreateFn;
	VuAssetFactory::AssetSchemaFn	mSchemaFn;
	VuAssetFactory::AssetBakeFn		mBakeFn;
	VUUINT32						mVersion;
	bool							mCompress;
};

//*****************************************************************************
// Macro used by registries to register assets.
#define REGISTER_ASSET(type, version, compression)															\
{																											\
	extern VuAsset *Create##type();																			\
	extern void type##Schema(const VuJsonContainer &, VuJsonContainer &);									\
	extern bool Bake##type(const VuJsonContainer &, VuAssetBakeParams &);									\
	VuAssetFactory::IF()->registerType(#type, Create##type, type##Schema, Bake##type, version, compression);\
}

#include "VuAssetFactory.inl"
