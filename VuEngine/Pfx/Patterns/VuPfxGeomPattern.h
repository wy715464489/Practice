//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Geom Pattern
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"
#include "VuEngine/Gfx/Model/VuStaticModelInstance.h"


class VuPfxGeomPattern : public VuPfxPattern
{
	DECLARE_RTTI
	DECLARE_PFX_PATTERN

public:
	VuPfxGeomPattern();

	virtual void			onLoad();

	void					modelAssetModified();

	// properties
	std::string				mModelAssetName;
	float					mRejectionScaleModifier;
	float					mNearFadeMin;
	float					mNearFadeMax;
	float					mFarFadeMin;
	float					mFarFadeMax;

	// model instance
	VuStaticModelInstance	mModelInstance;
};

class VuPfxGeomPatternInstance : public VuPfxPatternInstance
{
public:
	virtual void	start();
	virtual void	tick(float fdt, bool ui);
	virtual void	draw(const VuGfxDrawParams &params);
	virtual void	drawShadow(const VuGfxDrawShadowParams &params);
};
