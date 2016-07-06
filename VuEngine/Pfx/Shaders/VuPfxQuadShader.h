//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PfxQuadShader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuGfx.h"

class VuQuadShaderFlavor;
struct PfxQuadShaderDrawData;
class VuCamera;
class VuPfxQuadPatternInstance;
class VuShaderProgram;
class VuVertexDeclaration;
class VuGfxSortMaterial;

class VuPfxQuadShader
{
public:
	VuPfxQuadShader();
	~VuPfxQuadShader();

	bool					load();

	void					submit(const VuCamera &camera, const VuPfxQuadPatternInstance *pQuadPatternInstance);

protected:
	friend struct PfxQuadShaderDrawData;
	void					draw(const PfxQuadShaderDrawData *pDrawData);

private:
	VuQuadShaderFlavor		*mpFlavors;
};
