//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MeshInstance class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"

class VuGfxScene;
class VuGfxStaticSceneInfo;
class VuGfxSceneMesh;
class VuGfxSceneBakeState;
class VuMatrix;
class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuGfxSceneMeshInstance : public VuRefObj
{
protected:
	~VuGfxSceneMeshInstance();
public:
	VuGfxSceneMeshInstance();

	static bool				bake(const VuJsonContainer &data, VuGfxSceneBakeState &bakeState, VuBinaryDataWriter &writer);
	void					load(VuBinaryDataReader &reader);
	bool					fixup(const VuGfxScene *pScene);
	void					gatherSceneInfo(VuGfxStaticSceneInfo &sceneInfo, const VuMatrix &mat);

	int						mMeshIndex;
	VuGfxSceneMesh			*mpMesh;
};
