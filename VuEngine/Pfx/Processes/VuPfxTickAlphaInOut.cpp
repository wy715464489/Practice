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


class VuPfxTickAlphaInOut : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxTickAlphaInOut();

	float	mFadeInDuration;
	float	mFadeInRate;
	float	mFadeOutStartTime;
	float	mFadeOutRate;
};

class VuPfxTickAlphaInOutInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxTickAlphaInOut, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxTickAlphaInOut);


//*****************************************************************************
VuPfxTickAlphaInOut::VuPfxTickAlphaInOut():
	mFadeInDuration(1.0f),
	mFadeInRate(1.0f),
	mFadeOutStartTime(2.0f),
	mFadeOutRate(-1.0f)
{
	mProperties.add(new VuFloatProperty("Fade In Duration", mFadeInDuration));
	mProperties.add(new VuFloatProperty("Fade In Rate", mFadeInRate));
	mProperties.add(new VuFloatProperty("Fade Out Start Time", mFadeOutStartTime));
	mProperties.add(new VuFloatProperty("Fade Out Rate", mFadeOutRate));
}

//*****************************************************************************
void VuPfxTickAlphaInOutInstance::tick(float fdt, bool ui)
{
	const VuPfxTickAlphaInOut *pParams = static_cast<const VuPfxTickAlphaInOut *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
	{
		p->mColor.mW += VuSelect(pParams->mFadeInDuration - p->mAge, pParams->mFadeInRate*fdt, 0.0f);
		p->mColor.mW += VuSelect(pParams->mFadeOutStartTime - p->mAge, 0.0f, pParams->mFadeOutRate*fdt);
	}
}
