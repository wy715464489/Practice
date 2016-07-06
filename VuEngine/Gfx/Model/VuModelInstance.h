//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Model Instance
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Gfx/VuGfxDrawParams.h"

class VuGfxSceneInfo;
class VuGfxSceneMesh;
class VuGfxSortMaterial;
class VuMaterialSubstIF;


class VuModelInstance
{
public:
	VuModelInstance();
	virtual ~VuModelInstance();

	void				setColor(const VuColor &color)             { mColor = color; }
	void				setDynamicLightColor(const VuColor &color) { mDynamicLightColor = color; }
	void				setDynamicLightGroupMask(VUUINT32 mask)    { mDynamicLightGroupMask = mask; }
	void				enableTranslucentDepth(bool enable)        { mbTranslucentDepthEnabled = enable; }

	const VuColor		&getColor() const { return mColor; }

	void				setWaterZ(float waterZ)	{ mWaterZ = waterZ; }
	float				getWaterZ()	{ return mWaterZ; }

	void				setMaterialSubstIF(VuMaterialSubstIF *pMaterialSubstIF) { mpMaterialSubstIF = pMaterialSubstIF; }

protected:
	void				drawSceneInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params, const char *text) const;
	void				drawMeshInfo(VuGfxSceneMesh *pMesh, const VuMatrix &transform, const VuGfxDrawInfoParams &params) const;
	void				drawName(const char *strName, const VuAabb &aabb, const VuMatrix &worldMat, const VuGfxDrawInfoParams &params) const;

	VuColor				mColor;
	VuColor				mDynamicLightColor;
	VUUINT32			mDynamicLightGroupMask;
	bool				mbTranslucentDepthEnabled;
	float				mWaterZ;
	VuMaterialSubstIF	*mpMaterialSubstIF;
};


class VuMaterialSubstIF
{
public:
	virtual VuGfxSortMaterial *substituteMaterial(int flavor, int shaderIndex) = 0;
};