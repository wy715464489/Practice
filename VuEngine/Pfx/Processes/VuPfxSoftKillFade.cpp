//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Soft Kill Fade
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/VuPfxParticle.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuPfxSoftKillFade : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxSoftKillFade();

	float	mRate;
};

class VuPfxSoftKillFadeInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxSoftKillFade, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxSoftKillFade);


//*****************************************************************************
VuPfxSoftKillFade::VuPfxSoftKillFade() :
	mRate(-1.0f)
{
	mProperties.add(new VuFloatProperty("Rate", mRate));
}

//*****************************************************************************
void VuPfxSoftKillFadeInstance::tick(float fdt, bool ui)
{
	const VuPfxSoftKillFade *pParams = static_cast<const VuPfxSoftKillFade *>(mpParams);

	if (mpPatternInstance->mpSystemInstance->mState == VuPfxSystemInstance::STATE_STOPPING)
	{
		for (VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next())
		{
			p->mColor.mW += pParams->mRate*fdt;
		}
	}
}
