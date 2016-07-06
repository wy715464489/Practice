//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Recursive Pattern
// 
//*****************************************************************************

#include "VuPfxRecursivePattern.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxResources.h"
#include "VuEngine/Pfx/Particles/VuPfxRecursiveParticle.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"


IMPLEMENT_RTTI(VuPfxRecursivePattern, VuPfxPattern);
IMPLEMENT_PFX_PATTERN_REGISTRATION(VuPfxRecursivePattern, VuPfxRecursiveParticle);


//*****************************************************************************
VuPfxRecursivePattern::VuPfxRecursivePattern()
{
	// properties
	mProperties.add(new VuStringProperty("Child Pfx", mChildPfx));
}

//*****************************************************************************
void VuPfxRecursivePatternInstance::destroy()
{
	// destroy child pfx
	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);
		if ( pr->mpChildPfx )
			VuPfx::IF()->releaseSystemInstance(pr->mpChildPfx);
	}

	VuPfxPatternInstance::destroy();
}

//*****************************************************************************
void VuPfxRecursivePatternInstance::destroyParticles()
{
	// destroy child pfx
	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);
		if ( pr->mpChildPfx )
			VuPfx::IF()->releaseSystemInstance(pr->mpChildPfx);
	}

	VuPfxPatternInstance::destroyParticles();
}

//*****************************************************************************
VuPfxParticle *VuPfxRecursivePatternInstance::createParticle()
{
	VuPfxParticle *pParticle = VuPfxPatternInstance::createParticle();

	// create child pfx
	if ( pParticle )
	{
		const VuPfxRecursivePattern *pParams = static_cast<const VuPfxRecursivePattern *>(mpParams);
		VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(pParticle);
		pr->mpChildPfx = VuPfx::IF()->createSystemInstance(pParams->mChildPfx.c_str());
		if ( pr->mpChildPfx )
			pr->mpChildPfx->start();
	}

	return pParticle;
}

//*****************************************************************************
void VuPfxRecursivePatternInstance::start()
{
	for ( VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next() )
		p->start();
}

//*****************************************************************************
void VuPfxRecursivePatternInstance::tick(float fdt, bool ui)
{
	const VuPfxRecursivePattern *pParams = static_cast<const VuPfxRecursivePattern *>(mpParams);

	if ( mpSystemInstance->mCurrentTime > mpParams->mStartDelay )
	{
		// handle update
		VuPfxParticle *p = mParticles.front();
		while ( p )
		{
			// basic update
			p->mPosition += p->mLinearVelocity*fdt;
			p->mAge += fdt;

			// recursive-specific update
			VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);
			pr->mRotation += pr->mAngularVelocity*fdt;

			// tick child pfx
			if ( pr->mpChildPfx )
			{
				pr->mpChildPfx->mState = mpSystemInstance->mState;

				//pr->mpChildPfx->setLinearVelocity(p->mLinearVelocity);
				pr->mpChildPfx->setPosition(p->mPosition);
				pr->mpChildPfx->setRotation(pr->mRotation);
				pr->mpChildPfx->setColor(p->mColor);
				pr->mpChildPfx->tick(fdt, ui);
			}

			p = p->next();
		}

		for ( VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next() )
			p->tick(fdt, ui);

		// handle removal
		p = mParticles.front();
		while ( p )
		{
			VuPfxParticle *pNext = p->next();

			// removal
			if ( p->mAge > p->mLifespan )
			{
				// release child pfx
				VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);
				if ( pr->mpChildPfx )
					VuPfx::IF()->releaseSystemInstance(pr->mpChildPfx);

				mParticles.remove(p);
				VuPfx::IF()->resources()->freeParticle(p);
			}

			p = pNext;
		}
	}

	// update aabb using child pfx
	mAabb.reset();
	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);
		if ( pr->mpChildPfx )
			mAabb.addAabb(pr->mpChildPfx->getAabb());
	}

	if ( !mAabb.isValid() )
		mAabb = VuAabb(VuVector3(0,0,0), VuVector3(0,0,0));
}

//*****************************************************************************
void VuPfxRecursivePatternInstance::draw(const VuGfxDrawParams &params)
{
	VuPfxRecursivePattern *pParams = static_cast<VuPfxRecursivePattern *>(mpParams);

	const VuMatrix &drawMat = getDrawTransform();
	float scale = mpSystemInstance->mScale;
	VuVector4 vColor = mpSystemInstance->mColor;

	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);

		// draw child pfx
		if ( pr->mpChildPfx )
			pr->mpChildPfx->draw(params);
	}
}

//*****************************************************************************
void VuPfxRecursivePatternInstance::drawShadow(const VuGfxDrawShadowParams &params)
{
	const VuPfxRecursivePattern *pParams = static_cast<const VuPfxRecursivePattern *>(mpParams);

	VuMatrix drawMat = getDrawTransform();
	float scale = mpSystemInstance->mScale;
	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxRecursiveParticle *pr = static_cast<VuPfxRecursiveParticle *>(p);

		// draw child pfx shadow
		if ( pr->mpChildPfx )
			pr->mpChildPfx->drawShadow(params);
	}
}
