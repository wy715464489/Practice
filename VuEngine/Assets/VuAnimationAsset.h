//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"

class VuAnimation;


class VuAnimationAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuAnimationAsset() { unload(); }
public:
	VuAnimationAsset() : mpAnimation(VUNULL) {}

	VuAnimation			*getAnimation()			{ return mpAnimation; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	VuAnimation			*mpAnimation;
};
