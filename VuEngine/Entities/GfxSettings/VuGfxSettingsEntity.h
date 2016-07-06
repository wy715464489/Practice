//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSettings entity
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Gfx/VuGfxSettings.h"

class VuScriptComponent;


class VuGfxSettingsEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuGfxSettingsEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

	const VuGfxSettings	&getSettings()								{ return mSettings; }
	virtual float		getPositionalWeight(const VuVector3 &vPos)	{ return 0; }
	float				getTemporalWeight();

protected:
	void				tickBuild(float fdt);

	// scripting
	virtual VuRetVal	Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mbAlwaysActive;
	float				mRampUpTime;
	float				mDuration;
	float				mRampDownTime;
	VuGfxSettings		mSettings;

	float				mTimer;
};
