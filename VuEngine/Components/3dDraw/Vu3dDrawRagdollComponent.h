//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Vu3dDrawRagdollComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Gfx/Model/VuAnimatedModelInstance.h"
#include "VuEngine/Dynamics/VuRagdoll.h"

class VuPfxSystemInstance;
class VuDBEntryProperty;


class Vu3dDrawRagdollComponent : public Vu3dDrawComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(Ragdoll)
	DECLARE_RTTI

public:
	Vu3dDrawRagdollComponent(VuEntity *pOwnerEntity);
	~Vu3dDrawRagdollComponent();

	virtual void			onGameInitialize();
	virtual void			onGameRelease();

	void					setModelInstance(VuAnimatedModelInstance *pModelInstance);
	void					startSimulation(const VuAnimationTransform *pLocalPose, const VuVector3 &linVel, const VuVector3 &angVel);
	void					stopSimulation();

protected:
	void					draw(const VuGfxDrawParams &params);
	void					drawShadow(const VuGfxDrawShadowParams &params);

	virtual void			tickAnim(float fdt);

	// properties
	std::string				mRagdollType;
	std::string				mSplashPfx;
	float					mDrawDist;
	bool					mWaterSimulation;

	// property references
	VuDBEntryProperty		*mpRagdollTypeProperty;

	VuRagdoll				mRagdoll;
	VuAnimatedModelInstance	*mpModelInstance;
	VuPfxSystemInstance		*mpSplashPfx;
};
