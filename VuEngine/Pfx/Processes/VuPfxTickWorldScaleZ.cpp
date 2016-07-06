//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx WorldScaleZ Process
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxQuadParticle.h"
#include "VuEngine/Properties/VuPercentageProperty.h"


class VuPfxTickWorldScaleZ : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxTickWorldScaleZ();

	float	mStartDelay;
	float	mRate;
};

class VuPfxTickWorldScaleZInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxTickWorldScaleZ, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxTickWorldScaleZ);


//*****************************************************************************
VuPfxTickWorldScaleZ::VuPfxTickWorldScaleZ():
	mStartDelay(0.0f),
	mRate(0.0f)
{
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
	mProperties.add(new VuPercentageProperty("Rate", mRate));
}

//*****************************************************************************
void VuPfxTickWorldScaleZInstance::tick(float fdt, bool ui)
{
	const VuPfxTickWorldScaleZ *pParams = static_cast<const VuPfxTickWorldScaleZ *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
	{
		VuPfxQuadParticle *pq = static_cast<VuPfxQuadParticle *>(p);

		pq->mWorldScaleZ += VuSelect(pParams->mStartDelay - p->mAge, 0.0f, pParams->mRate*fdt);
	}
}
