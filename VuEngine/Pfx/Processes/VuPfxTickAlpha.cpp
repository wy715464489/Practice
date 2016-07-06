//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Alpha Process
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/VuPfxParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuPfxTickAlpha : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxTickAlpha();

	float	mStartDelay;
	float	mRate;
};

class VuPfxTickAlphaInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxTickAlpha, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxTickAlpha);


//*****************************************************************************
VuPfxTickAlpha::VuPfxTickAlpha():
	mStartDelay(0.0f),
	mRate(-1.0f)
{
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
	mProperties.add(new VuFloatProperty("Rate", mRate));
}

//*****************************************************************************
void VuPfxTickAlphaInstance::tick(float fdt, bool ui)
{
	const VuPfxTickAlpha *pParams = static_cast<const VuPfxTickAlpha *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
	{
		p->mColor.mW += VuSelect(pParams->mStartDelay - p->mAge, 0.0f, pParams->mRate*fdt);
	}
}
