//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Material Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"

class VuGfxSortMaterial;
class VuGfxSortMaterialDesc;
class VuMatrix;
class VuColor;
class VuAabb;


class VuMaterialAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuMaterialAsset() { unload(); }
public:

	VuMaterialAsset();

	static void				schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool				bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

	static bool				loadShaderData(const std::string &shaderFileName, VuJsonContainer &shaderData);
	static void				buildParameterMacros(const VuJsonContainer &materialData, const VuJsonContainer &defParent, VuShaderProgram::Macros &macros);

protected:
	static void				buildSchema(const VuJsonContainer &materialData, const VuJsonContainer &defParent, VuJsonContainer &schema);

	virtual bool			load(VuBinaryDataReader &reader);
	virtual void			unload();

	void					resolveConstants();
	static void				buildMaterialDesc(const VuJsonContainer &materialData, const VuJsonContainer &defParent, VuGfxSortMaterialDesc &desc, std::string &errors);

public:
	enum eFlavor { FLV_OPAQUE, FLV_MODULATED, FLV_ADDITIVE, FLV_DEPTH, NUM_FLAVORS };

	void					setModelMatrix(const VuMatrix &modelMat) const;
	void					setMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const;
	void					setColor(const VuColor &color) const;
	void					setWaterZ(float waterZ) const;
	void					setDynamicLightColor(const VuColor &color) const;
	void					setDynamicLights(const VuMatrix &transform, const VuAabb &aabb, VUUINT32 groupMask) const;

	void					setShadowMatrix(const VuMatrix &mat) const;
	void					setShadowMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const;

	void					setDropShadowMatrix(const VuMatrix &modelMat) const;
	void					setDropShadowMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const;

	void					setSSAODepthModelMatrix(const VuMatrix &modelMat) const;
	void					setSSAODepthMatrixArray(const VuMatrix *pMatrixArray, int boneCount) const;

	static eFlavor			getFlavor(int translucencyType);

	bool					mHasShaderLODs;
	bool					mbAlphaTest;
	bool					mbSkinning;
	bool					mbDoesCastShadows;
	bool					mbDoesReceiveShadows;
	bool					mbDoesSSAO;
	bool					mbSceneLighting;
	bool					mbDynamicLighting;
	bool					mbDepthSort;
	VUUINT32				mTranslucencyType;

	VuGfxSortMaterial		*mpGfxSortMaterials[NUM_FLAVORS];
	VuGfxSortMaterial		*mpGfxSortShadowMaterial;
	VuGfxSortMaterial		*mpGfxSortDropShadowMaterial;
	VuGfxSortMaterial		*mpGfxSortSSAODepthMaterial;

	VuShaderProgram			*mpShaderProgram;
	VuShaderProgram			*mpShadowShaderProgram;
	VuShaderProgram			*mpDropShadowShaderProgram;
	VuShaderProgram			*mpSSAODepthShaderProgram;

	// constants
	struct Constants
	{
		VUHANDLE	mhModelMatrix;
		VUHANDLE	mhMatrixArray;
		VUHANDLE	mhColor;
		VUHANDLE	mhWaterZ;
		VUHANDLE	mhDynamicLightColor;
		VUHANDLE	mhDynamicLightDirections;
		VUHANDLE	mhDynamicLightDiffuseColors;
	};
	Constants				mConstants;

	struct ShadowConstants
	{
		VUHANDLE	mhMatrix;
		VUHANDLE	mhMatrixArray;
	};
	ShadowConstants			mShadowConstants;

	struct DropShadowConstants
	{
		VUHANDLE	mhMatrix;
		VUHANDLE	mhMatrixArray;
	};
	DropShadowConstants		mDropShadowConstants;

	struct SSAODepthConstants
	{
		VUHANDLE	mhModelMatrix;
		VUHANDLE	mhMatrixArray;
	};
	SSAODepthConstants		mSSAODepthConstants;
};
