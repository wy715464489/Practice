//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PfxTrailShader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Pfx/Patterns/VuPfxTrailPattern.h"

struct PfxTrailShaderDrawData;
class VuCamera;
class VuPfxTrailPatternInstance;
class VuCompiledShaderAsset;
class VuVertexDeclaration;
class VuGfxSortMaterial;


class VuPfxTrailShader
{
public:
	VuPfxTrailShader();
	~VuPfxTrailShader();

	bool					load();

	void					submit(const VuCamera &camera, const VuPfxTrailPatternInstance *pTrailPatternInstance);

protected:
	friend struct PfxTrailShaderDrawData;
	void					draw(const PfxTrailShaderDrawData *pDrawData);

private:
	VuGfxSortMaterial		*mpMaterials[VuPfxTrailPattern::BLEND_MODE_COUNT];

	struct Constants
	{
		VUINT	miColorSampler;
	};
	Constants				mConstants;
};
