//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Scene class
// 
//*****************************************************************************

#pragma once

#include "VuGfxScene.h"

class VuGfxSceneNode;
class VuAssetBakeParams;


class VuGfxStaticScene : public VuGfxScene
{
protected:
	~VuGfxStaticScene() { clear(); }
public:
	VuGfxStaticScene();

	void	clear();

	bool	load(const VuJsonContainer &data);
	bool	load(VuBinaryDataReader &reader);

	static bool	bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams, const VuJsonContainer &data, bool flipX, VuBinaryDataWriter &writer);

	typedef std::list<VuGfxSceneNode *> Nodes;

	Nodes					mNodes;
	VuGfxStaticSceneInfo	mStaticInfo;

protected:
	void	gatherSceneInfo();
};