//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic light entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"

class Vu3dLayoutDrawParams;
class VuScriptComponent;
class Vu3dLayoutComponent;


class VuDynamicLightEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuDynamicLightEntity();

	virtual void		onPostLoad() { modified(); }
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

	const VuDynamicLight	&dynamicLight() const { return mDynamicLight; }

private:
	// scripting
	VuRetVal			TurnOn(const VuParams &params)	{ mDynamicLight.turnOn(); return VuRetVal(); }
	VuRetVal			TurnOff(const VuParams &params)	{ mDynamicLight.turnOff(); return VuRetVal(); }

	void				drawLayout(const Vu3dLayoutDrawParams &params);
	void				modified();

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	// properties
	bool				mbInitiallyOn;
	VuDynamicLight		mDynamicLight;
};
