//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water Light Map Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Containers/VuArray.h"


class VuLightMapAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuLightMapAsset() { unload(); }
public:
	VuLightMapAsset();

	int					getWidth()	{ return mWidth; }
	int					getHeight()	{ return mHeight; }
	VuArray<VUUINT16>	&getRGB16()	{ return mRGB16; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	int					mWidth;
	int					mHeight;
	VuArray<VUUINT16>	mRGB16;
};
