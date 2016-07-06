//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneNode class
// 
//*****************************************************************************

#pragma once

#include "VuGfxSceneInfo.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Objects/VuRefObj.h"

class VuGfxScene;
class VuGfxSceneMeshInstance;
class VuGfxSceneBakeState;
class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;

class VuGfxSceneNode : public VuRefObj
{
protected:
	~VuGfxSceneNode();
public:
	VuGfxSceneNode();

	static bool				bake(const VuJsonContainer &data, VuGfxSceneBakeState &bakeState, bool flipX, VuBinaryDataWriter &writer);
	void					load(VuBinaryDataReader &reader);
	bool					fixup(const VuGfxScene *pScene, const VuMatrix &mat);
	void					gatherSceneInfo(VuGfxStaticSceneInfo &sceneInfo, const VuMatrix &mat);
	void					calculateAabbRecursive(VuAabb &aabb, const VuMatrix &mat);

	typedef std::list<VuGfxSceneNode *> Children;

	std::string				mstrName;
	VuMatrix				mTransform;
	VuGfxSceneMeshInstance	*mpMeshInstance;
	Children				mChildren;
	VuAabb					mAabb;
};
