//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Recursive Pattern
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"
#include "VuEngine/Gfx/Model/VuStaticModelInstance.h"


class VuPfxRecursivePattern : public VuPfxPattern
{
	DECLARE_RTTI
	DECLARE_PFX_PATTERN

public:
	VuPfxRecursivePattern();

	// properties
	std::string		mChildPfx;
};

class VuPfxRecursivePatternInstance : public VuPfxPatternInstance
{
public:
	virtual void			destroy();
	virtual void			destroyParticles();
	virtual void			start();
	virtual void			tick(float fdt, bool ui);
	virtual void			draw(const VuGfxDrawParams &params);
	virtual void			drawShadow(const VuGfxDrawShadowParams &params);
	virtual VuPfxParticle	*createParticle();
};
