//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Linear Acceleration Process
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/VuPfxParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuPfxTickDampenVelocity : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxTickDampenVelocity();

	float	mAmount;
	float	mStartDelay;
};

class VuPfxTickDampenVelocityInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxTickDampenVelocity, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxTickDampenVelocity);


//*****************************************************************************
VuPfxTickDampenVelocity::VuPfxTickDampenVelocity():
	mAmount(0),
	mStartDelay(0.0f)
{
	mProperties.add(new VuFloatProperty("Amount", mAmount));
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
}

//*****************************************************************************
void VuPfxTickDampenVelocityInstance::tick(float fdt, bool ui)
{
	const VuPfxTickDampenVelocity *pParams = static_cast<const VuPfxTickDampenVelocity *>(mpParams);

	float fAmount = VuMin(pParams->mAmount*fdt, 1.0f);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
		if ( p->mAge > pParams->mStartDelay )
			p->mLinearVelocity *= (1.0f - fAmount);
}
