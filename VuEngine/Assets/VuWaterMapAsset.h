//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water Map Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Containers/VuArray.h"


class VuWaterMapAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuWaterMapAsset() { unload(); }
public:
	VuWaterMapAsset();

	class VuClipLevel;

	int					getWidth()		{ return mWidth; }
	int					getHeight()		{ return mHeight; }
	VuArray<VUUINT16>	&getSFD16()		{ return mSFD16; }
	bool				isVisible(int level, int x, int y);

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	typedef VuArray<VuClipLevel *> ClipLevels;

	int					mWidth;
	int					mHeight;
	VuArray<VUUINT16>	mSFD16;
	ClipLevels			mClipLevels;
};
