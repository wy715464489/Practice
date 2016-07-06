//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic Data Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Json/VuJsonContainer.h"


class VuGenericDataAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuGenericDataAsset() { unload(); }
public:

	const VuJsonContainer	&getDataContainer() const { return mDataContainer; }

	static bool				bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

protected:
	virtual bool			load(VuBinaryDataReader &reader);
	virtual void			unload();

	VuJsonContainer			mDataContainer;
};
