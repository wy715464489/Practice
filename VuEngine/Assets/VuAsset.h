//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAssetDependencies.h"
#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


class VuAsset
{
	DECLARE_RTTI

protected:
	friend class VuAssetFactory;
	VuAsset() : mHashID(0), mRefCount(1), mChildAssets(0) {}
	virtual ~VuAsset() {}

	void				addRef()	{ mRefCount++; }
	void				removeRef()	{ mRefCount--; }
	VUINT32				refCount()	{ return mRefCount; }
public:
	typedef VuArray<VUUINT32> ChildAssets;

	void				setAssetName(const std::string &assetName, VUUINT32 hashID)	{ mAssetName = assetName; mHashID = hashID; }
	const std::string	&getAssetName() const	{ return mAssetName; }
	VUUINT32			getHashID() const		{ return mHashID; }
	const ChildAssets	&childAssets()			{ return mChildAssets; }

	virtual bool		substitute(const VuAsset *pSubstAsset) { return true; }
	virtual void		editorReload();

private:
	virtual bool		load(VuBinaryDataReader &reader) = 0;
	virtual void		unload() = 0;

	std::string			mAssetName;
	VUUINT32			mHashID;
	VUINT32				mRefCount;
	ChildAssets			mChildAssets;
};

class VuAssetBakeParams
{
public:
	VuAssetBakeParams(const std::string &platform, const std::string &sku, const std::string &language);

	// input
	std::string	mPlatform;
	std::string	mSku;
	std::string	mLanguage;

	// output
	VuArray<VUBYTE>		mData;
	VuAssetDependencies	mDependencies;

	// utility
	VuBinaryDataWriter	mWriter;
};

//*****************************************************************************
// Macro used by the various entity implementations to declare their creation
// functions.
#define IMPLEMENT_ASSET_REGISTRATION(type) \
	VuAsset *Create##type() { return new type(); } \
	void type##Schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema) { type::schema(creationInfo, schema); } \
	bool Bake##type(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams) { return type::bake(creationInfo, bakeParams); }
