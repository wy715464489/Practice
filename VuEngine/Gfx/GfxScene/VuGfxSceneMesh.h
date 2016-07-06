//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Mesh class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuAabb.h"

class VuGfxScene;
class VuGfxSceneInfo;
class VuGfxSceneMeshPart;
class VuGfxSceneBakeState;
class VuJsonContainer;
class VuAabb;
class VuGfxSortMesh;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuGfxSceneMesh : public VuRefObj
{
protected:
	~VuGfxSceneMesh();
public:
	VuGfxSceneMesh();

	void				load(VuBinaryDataReader &reader);
	static bool			bake(const VuJsonContainer &data, int meshIndex, VuGfxSceneBakeState &bakeState, int vertexStride, bool bSkinning, bool flipX, VuBinaryDataWriter &writer);
	bool				fixup(const VuGfxScene *pScene);
	void				gatherSceneInfo(VuGfxSceneInfo &sceneInfo);
	const VuAabb		&getAabb() const { return mAabb; }

	typedef std::list<VuGfxSceneMeshPart *> Parts;

	std::string			mstrName;
	Parts				mParts;
	VuAabb				mAabb;
};
