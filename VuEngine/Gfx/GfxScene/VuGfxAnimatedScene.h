//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Scene class
// 
//*****************************************************************************

#pragma once

#include "VuGfxScene.h"

class VuAssetBakeParams;


class VuGfxAnimatedScene : public VuGfxScene
{
protected:
	~VuGfxAnimatedScene() { clear(); }
public:
	VuGfxAnimatedScene();

	void	clear();

	bool	load(const VuJsonContainer &data);
	bool	load(VuBinaryDataReader &reader);

	static bool	bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams, const VuJsonContainer &data, VuBinaryDataWriter &writer);

	VuGfxAnimatedSceneInfo	mAnimatedInfo;

protected:
	void	gatherSceneInfo();
};
