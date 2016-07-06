//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Lens Water Manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuTexture;
class VuRenderTarget;
class VuVector3;


class VuLensWaterManager : VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuLensWaterManager)

protected:
	// called by game
	friend class VuEngine;
	virtual bool init() = 0;

public:
	virtual bool		isEnabled() = 0;
	virtual bool		isActive(int viewport) = 0;

	virtual void		reset() = 0;

	virtual void		addDroplets(int viewport, float count) = 0;
	virtual void		setRadialSpread(int viewport, float amount) = 0;

	virtual void		setViewportCount(int count) = 0;
	virtual void		updateTextureSize(int viewport, int width, int height) = 0;
	virtual void		submit(int viewport, VuTexture *pSourceTexture, VuRenderTarget *pRenderTarget) = 0;

	class VuEmitterIF { public: virtual float lensWaterRate(const VuVector3 &pos) = 0; };
	virtual void		registerEmitter(VuEmitterIF *pEmitter) = 0;
	virtual void		unregisterEmitter(VuEmitterIF *pEmitter) = 0;
};