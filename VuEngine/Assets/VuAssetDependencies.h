//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Asset dependencies class
// 
//*****************************************************************************

#pragma once

class VuBinaryDataWriter;
class VuBinaryDataReader;


class VuAssetDependencies
{
public:
	void	addAsset(const std::string &type, const std::string &name);
	void	addFile(const std::string &fileName);
	bool	finalize(const std::string &platform, const std::string &sku, const std::string &language);
	void	serialize(VuBinaryDataWriter &writer) const;
	void	deserialize(VuBinaryDataReader &reader);
	bool	check(const std::string &language);
	bool	check(const std::string &platform, const std::string &sku, const std::string &language);

	struct VuAssetEntry
	{
		std::string	mType;
		std::string	mName;
		VUUINT32	mInfoHash;
		VUUINT32	mDataHash;
	};

	struct VuFileEntry
	{
		std::string	mFileName;
		VUUINT32	mHash;
	};

	int					assetCount() const { return (int)mAssetEntries.size(); }
	const VuAssetEntry	&assetEntry(int index) const { return mAssetEntries[index]; }

private:
	typedef std::vector<VuAssetEntry> AssetEntries;
	typedef std::vector<VuFileEntry> FileEntries;

	AssetEntries	mAssetEntries;
	FileEntries		mFileEntries;
};
