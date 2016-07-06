//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SceneInfo class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuAabb.h"


class VuGfxSceneInfo
{
public:
	VuGfxSceneInfo() : mNumMeshes(0), mNumMeshParts(0), mNumVerts(0), mNumTris(0), mNumMaterials(0) {}

	int			mNumMeshes;				// number of stored meshes (meshes contain vertex data)
	int			mNumMeshParts;			// number of mesh instance parts (parts contain index data)
	int			mNumVerts;				// number of stored verts (does not account for instancing)
	int			mNumTris;				// number of stored tris (does not account for instancing)
	int			mNumMaterials;			// number of materials
};

class VuGfxStaticSceneInfo
{
public:
	VuGfxStaticSceneInfo() : mNumNodes(0), mNumMeshInstances(0), mNumDrawnVerts(0), mNumDrawnTris(0) {}

	int			mNumNodes;				// number of nodes
	int			mNumMeshInstances;		// number of mesh instances
	int			mNumDrawnVerts;			// number of verts drawn (accounts for instancing)
	int			mNumDrawnTris;			// number of tris drawn (accounts for instancing)
	VuAabb		mAabb;					// axis aligned bounding box for scene
};

class VuGfxAnimatedSceneInfo
{
public:
	VuGfxAnimatedSceneInfo() {}

	VuAabb		mAabb;					// axis aligned bounding box for scene
};