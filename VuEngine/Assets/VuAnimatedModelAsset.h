//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Model Asset class
// 
//*****************************************************************************

#pragma once

#include "VuAsset.h"

class VuSkeleton;
class VuGfxAnimatedScene;


class VuAnimatedModelAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuAnimatedModelAsset() { unload(); }
public:
	VuAnimatedModelAsset() : mpGfxAnimatedScene(VUNULL),mpSkeleton(VUNULL) {}

	VuSkeleton			*getSkeleton()			{ return mpSkeleton; }
	VuGfxAnimatedScene	*getGfxAnimatedScene()	{ return mpGfxAnimatedScene; }

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();
	virtual void		editorReload();

	VuGfxAnimatedScene	*mpGfxAnimatedScene;
	VuSkeleton			*mpSkeleton;
};
