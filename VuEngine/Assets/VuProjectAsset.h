//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Project Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"


class VuProjectAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuProjectAsset() {}

public:
	const VuJsonContainer	&getProject() const { return mDataContainer; }
	const std::string		&getProjectName() const { return mProjectName; }

	static void				schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool				bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

	int						getAssetCount();
	void					getAssetInfo(int index, std::string &assetType, std::string &assetName);

private:
	virtual bool			load(VuBinaryDataReader &reader);
	virtual void			unload();

	VuJsonContainer			mDataContainer;
	std::string				mProjectName;

};
