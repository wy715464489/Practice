//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"
#include "VuEngine/Containers/VuArray.h"


class VuGenericAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuGenericAsset() { unload(); }
public:
	const VuArray<VUBYTE>	&data() const	{ return mData; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

protected:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

private:
	VuArray<VUBYTE>		mData;
};
