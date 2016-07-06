//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Scale Process
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/VuPfxParticle.h"
#include "VuEngine/Properties/VuPercentageProperty.h"


class VuPfxTickScale : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxTickScale();

	float	mStartDelay;
	float	mRate;
};

class VuPfxTickScaleInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxTickScale, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxTickScale);


//*****************************************************************************
VuPfxTickScale::VuPfxTickScale():
	mStartDelay(0.0f),
	mRate(0.0f)
{
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
	mProperties.add(new VuPercentageProperty("Rate", mRate));
}

//*****************************************************************************
void VuPfxTickScaleInstance::tick(float fdt, bool ui)
{
	const VuPfxTickScale *pParams = static_cast<const VuPfxTickScale *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
	{
		p->mScale += VuSelect(pParams->mStartDelay - p->mAge, 0.0f, pParams->mRate*fdt);
	}
}
