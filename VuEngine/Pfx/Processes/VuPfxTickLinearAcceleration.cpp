//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
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


class VuPfxTickLinearAcceleration : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxTickLinearAcceleration();

	VuVector3	mAccel;
	float		mStartDelay;
};

class VuPfxTickLinearAccelerationInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxTickLinearAcceleration, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxTickLinearAcceleration);


//*****************************************************************************
VuPfxTickLinearAcceleration::VuPfxTickLinearAcceleration():
	mAccel(0,0,0),
	mStartDelay(0.0f)
{
	mProperties.add(new VuFloatProperty("Accel X", mAccel.mX));
	mProperties.add(new VuFloatProperty("Accel Y", mAccel.mY));
	mProperties.add(new VuFloatProperty("Accel Z", mAccel.mZ));
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
}

//*****************************************************************************
void VuPfxTickLinearAccelerationInstance::tick(float fdt, bool ui)
{
	const VuPfxTickLinearAcceleration *pParams = static_cast<const VuPfxTickLinearAcceleration *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
		if ( p->mAge > pParams->mStartDelay )
			p->mLinearVelocity += pParams->mAccel*fdt;
}
