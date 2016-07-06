//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Model Skin class
// 
//*****************************************************************************

#pragma once

#include "VuStaticModelInstance.h"
#include "VuEngine/Containers/VuArray.h"

class VuGfxScene;
class VuJsonContainer;


class VuModelSkin : public VuMaterialSubstIF
{
public:
	VuModelSkin() : mShaderCount(0) {}
	~VuModelSkin();

	void	clear();
	void	build(const VuGfxScene *pGfxScene, const VuJsonContainer &data);

private:
	// VuMaterialSubstIF
	virtual VuGfxSortMaterial	*substituteMaterial(int flavor, int shaderIndex) { return mSkinMaterials[flavor*mShaderCount + shaderIndex]; }

	typedef VuArray<VuGfxSortMaterial *> SkinMaterials;

	int				mShaderCount;
	SkinMaterials	mSkinMaterials;
};
