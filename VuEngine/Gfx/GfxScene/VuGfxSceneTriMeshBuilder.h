//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSceneTriMeshBuilder class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuObjectArray.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuColor.h"

class VuJsonContainer;
class VuMatrix;
class VuCollisionMaterialAsset;


class VuGfxSceneTriMeshBuilder
{
public:
	VuGfxSceneTriMeshBuilder(const std::string &platform, const std::string &sku, const std::string &language, const VuJsonContainer &creationInfo, const VuJsonContainer &data);
	~VuGfxSceneTriMeshBuilder();

	void		build(bool flipX);

	struct Triangle
	{
		int		mMaterialIndex;
		VuColor	mColor0;
		VuColor	mColor1;
		VuColor	mColor2;
	};
	typedef VuObjectArray<Triangle> Tris;

	typedef VuObjectArray<int> Indices;
	typedef VuObjectArray<VuVector3> Verts;
	typedef VuObjectArray<VuColor> Colors;
	struct Mesh
	{
		const VuJsonContainer	*mpPartsData;
		Verts					mVerts;
		Colors					mColors;
		Indices					mIndices;
	};
	typedef std::map<std::string, Mesh> Meshes;
	struct Material
	{
		std::string	mName;
		std::string	mAssetName;
		std::string	mSurfaceType;
		float		mHardEdgeThreshold;
		bool		mCoronaCollision;
		bool		mReceiveShadows;
		bool		mIgnoreBakedShadows;
		Indices		mIndices;
		Tris		mTris;
	};
	typedef std::vector<Material> Materials;

	void					buildMesh(const VuJsonContainer &data, Mesh &mesh);
	void					gatherTrisRecursive(const VuJsonContainer &data, const VuMatrix &transform);
	void					addTris(const VuJsonContainer &data, const Mesh &mesh, const VuMatrix &transform);

	std::string				mPlatform;
	std::string				mSku;
	std::string				mLanguage;
	const VuJsonContainer	&mCreationInfo;
	const VuJsonContainer	&mData;

	Verts					mVerts;
	Indices					mIndices;
	Tris					mTris;

	Meshes					mMeshes;
	Materials				mMaterials;
};