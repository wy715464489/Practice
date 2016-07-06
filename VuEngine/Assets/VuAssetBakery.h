//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset Baker class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Json/VuJsonContainer.h"

class VuAssetPackFileWriter;
class VuAssetBakeParams;
class VuAssetDependencies;
struct VuBakedAssetHeader;


// compression methods
enum eVuAssetCompressionType { VU_ASSET_COMPRESSION_OFF, VU_ASSET_COMPRESSION_ZLIB, VU_ASSET_COMPRESSION_LZMA };


class VuAssetBakery : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuAssetBakery)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();

public:
	bool	reloadAssetInfo();

	bool	bakeAsset(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName);
	bool	packAsset(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, VuAssetPackFileWriter &packFileWriter);
	bool	packAssetDefinitions(const std::string &platform, const std::string &sku, VuAssetPackFileWriter &packFileWriter);
	bool	getAssetDataHash(const std::string &platform, const std::string &sku, const std::string &language, const std::string &assetType, const std::string &assetName, VUUINT32 &hash);

	enum eLoadBakedFileMode { LBFM_HEADER, LBFM_COMPRESSED_DATA, LBFM_UNCOMPRESSED_DATA };
	static bool	needToBake(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, VUUINT32 creationInfoHash);
	static bool	loadBakedFile(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, VUUINT32 creationInfoHash, VuBakedAssetHeader &header, VuAssetDependencies &dependencies, VuArray<VUBYTE> &data, eLoadBakedFileMode mode);
	static bool	saveBakedFile(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, VUUINT32 creationInfoHash, const VuAssetBakeParams &bakeParams);
	static bool	cacheBakedFile(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang);
	static void	getBakedFileName(const std::string &platform, const std::string &sku, const std::string &assetType, const std::string &assetName, const std::string &lang, std::string &strBakedFileName);

	bool							doesAssetExist(const std::string &assetType, const std::string &assetName);
	VUUINT32						getAssetInfoHash(const std::string &assetType, const std::string &assetName);
	const VuJsonContainer			&getCreationInfo(const std::string &platform, const std::string &sku, const std::string &language, const std::string &assetType, const std::string &assetName);
	static const VuJsonContainer	&getCreationInfo(const std::string &platform, const std::string &sku, const VuJsonContainer &assetData);

	// manipulation
	void	editorSetAssetData(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData);
	void	editorCreateAsset(const std::string &strType, const std::string &strAsset, const VuJsonContainer &assetData);
	void	editorDeleteAsset(const std::string &strType, const std::string &strAsset);

private:
	const VuJsonContainer &assetDB() { return mAssetDB; }

	VuJsonContainer	mAssetDB;
};

struct VuBakedAssetHeader
{
	VUUINT32	mMagic;
	VUUINT32	mHeaderVersion;
	VUUINT32	mAssetVersion;
	VUUINT32	mUncompressedDataSize;
	VUUINT32	mCompressedDataSize;
	VUUINT32	mCreationInfoHash;
	VUUINT32	mDataHash;
	VUUINT16	mDependencySize;
	VUUINT16	mCompressionType;

	void flipEndianness();
};
