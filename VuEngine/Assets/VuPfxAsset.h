//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"


class VuPfxAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuPfxAsset() { unload(); }

public:
	static void		schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool		bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

protected:
	virtual bool	load(VuBinaryDataReader &reader);
	virtual void	unload();
};
