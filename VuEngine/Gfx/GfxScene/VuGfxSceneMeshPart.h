//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MeshPart class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Math/VuAabb.h"

class VuGfxScene;
class VuGfxSceneMaterial;
class VuGfxSceneChunk;
class VuGfxSceneInfo;
class VuBinaryDataReader;


class VuGfxSceneMeshPart : public VuRefObj
{
protected:
	~VuGfxSceneMeshPart();
public:
	VuGfxSceneMeshPart();

	void				load(VuBinaryDataReader &reader);
	bool				fixup(const VuGfxScene *pScene);
	void				gatherSceneInfo(VuGfxSceneInfo &sceneInfo);

	VuGfxSceneMaterial	*mpMaterial;
	VuGfxSceneChunk		*mpChunk;
	int					mMaterialIndex;
	int					mChunkIndex;
	int					mMinIndex;
	int					mMaxIndex;
	int					mVertCount;
	int					mStartIndex;
	int					mTriCount;
	VuAabb				mAabb;
};
