//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Spring Constraint Process
// 
//*****************************************************************************

#include "VuPfxSpringConstraint.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/VuPfxParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"


IMPLEMENT_RTTI(VuPfxSpringConstraint, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxSpringConstraint);


//*****************************************************************************
VuPfxSpringConstraint::VuPfxSpringConstraint():
	mSpringCoeff(1),
	mDampingCoeff(1),
	mStartDelay(0),
	mTarget(0,0,0)
{
	mProperties.add(new VuFloatProperty("Spring Coeff", mSpringCoeff));
	mProperties.add(new VuFloatProperty("Damping Coeff", mDampingCoeff));
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
	mProperties.add(new VuFloatProperty("Target X", mTarget.mX));
	mProperties.add(new VuFloatProperty("Target Y", mTarget.mY));
	mProperties.add(new VuFloatProperty("Target Z", mTarget.mZ));
}

//*****************************************************************************
void VuPfxSpringConstraintInstance::tick(float fdt, bool ui)
{
	const VuPfxSpringConstraint *pParams = static_cast<const VuPfxSpringConstraint *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
	{
		if ( p->mAge > pParams->mStartDelay )
		{
			VuVector3 acc = pParams->mSpringCoeff*(pParams->mTarget + mTargetOffset - p->mPosition) - pParams->mDampingCoeff*p->mLinearVelocity;
			p->mLinearVelocity += acc*fdt;
		}
	}
}
