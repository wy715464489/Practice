//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Collision Material Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"


class VuCollisionMaterialAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuCollisionMaterialAsset() { unload(); }
public:
	VuCollisionMaterialAsset();

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

	std::string			mSurfaceType;
	float				mHardEdgeThreshold;
	bool				mCoronaCollision;
	bool				mReceiveShadows;
	bool				mIgnoreBakedShadows;

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();
};
