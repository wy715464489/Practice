//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Scene class
// 
//*****************************************************************************

#pragma once

#include "VuGfxSceneInfo.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"

class VuGfxSceneMesh;
class VuGfxSceneMaterial;
class VuGfxSceneChunk;
class VuGfxSceneBakeState;
class VuVertexBuffer;
class VuIndexBuffer;
class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuAssetDependencies;
class VuAssetBakeParams;

class VuGfxScene : public VuRefObj
{
protected:
	~VuGfxScene() { clear(); }
public:
	VuGfxScene();
	void	clear();

	typedef std::vector<VuGfxSceneMesh *> Meshes;
	typedef std::vector<VuGfxSceneMaterial *> Materials;
	typedef std::vector<VuGfxSceneChunk *> Chunks;

	Materials			mMaterials;
	Meshes				mMeshes;
	Chunks				mChunks;
	VuGfxSceneInfo		mInfo;

protected:
	bool				load(VuBinaryDataReader &reader);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams, const VuJsonContainer &data, VuGfxSceneBakeState &bakeState, bool bSkinning, bool flipX, VuBinaryDataWriter &writer);
	static void			optimizeVerts(std::vector<VUBYTE> &vertexData, int oldStride, int newStride);
	void				gatherSceneInfo();
};