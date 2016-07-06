//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Ragdoll Splash Emitter
// 
//*****************************************************************************

#include "VuEngine/Pfx/Processes/VuPfxEmitFountain.h"

class VuRagdoll;


class VuPfxEmitRagdollSplashQuadFountain : public VuPfxEmitQuadFountain
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxEmitRagdollSplashQuadFountain();

	float		mMinEmitVel;
};

class VuPfxEmitRagdollSplashQuadFountainInstance : public VuPfxEmitQuadFountainInstance
{
public:
	VuPfxEmitRagdollSplashQuadFountainInstance() : mpRagdoll(VUNULL) {}

	virtual void	tick(float fdt, bool ui);

	const VuRagdoll	*mpRagdoll;
};
