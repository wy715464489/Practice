//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx System
// 
//*****************************************************************************

#include "VuPfxSystem.h"
#include "VuPfx.h"
#include "VuPfxResources.h"
#include "VuPfxRegistry.h"
#include "VuPfxPattern.h"
#include "VuPfxProcess.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Dev/VuDevProfile.h"


IMPLEMENT_RTTI(VuPfxSystem, VuPfxNode);


//*****************************************************************************
VuPfxSystem::VuPfxSystem():
	mDuration(0.0f)
{
	// properties
	mProperties.add(new VuFloatProperty("Duration", mDuration));
}

//*****************************************************************************
VuPfxSystemInstance::VuPfxSystemInstance():
	mMatrix(VuMatrix::identity()),
	mLinearVelocity(0,0,0),
	mRotation(0,0,0),
	mState(STATE_STOPPED),
	mAabb(VuVector3(0,0,0), VuVector3(0,0,0)),
	mParticleCount(0),
	mCurrentTime(0.0f),
	mScale(1.0f),
	mColor(1,1,1,1)
{
}

//*****************************************************************************
bool VuPfxSystemInstance::create()
{
	for ( VuPfxSystem::ChildNodes::iterator iter = mpParams->mChildNodes.begin(); iter != mpParams->mChildNodes.end(); iter++ )
	{
		VuPfxPatternInstance *pPatternInstance = VuPfx::IF()->resources()->allocatePattern(static_cast<VuPfxPattern *>(iter->second));
		if ( pPatternInstance == VUNULL )
			return false;

		pPatternInstance->mpSystemInstance = this;

		if ( !pPatternInstance->create() )
		{
			pPatternInstance->destroy();
			VuPfx::IF()->resources()->freePattern(pPatternInstance);
			return false;
		}

		mPatterns.push_back(pPatternInstance);
	}

	return true;
}

//*****************************************************************************
void VuPfxSystemInstance::destroy()
{
	while ( VuPfxPatternInstance *pPatternInstance = mPatterns.pop_back() )
	{
		pPatternInstance->destroy();
		VuPfx::IF()->resources()->freePattern(pPatternInstance);
	}
}

//*****************************************************************************
void VuPfxSystemInstance::start()
{
	mState = STATE_ALIVE;
	mCurrentTime = 0.0f;

	for ( VuPfxPatternInstance *p = mPatterns.front(); p; p = p->next() )
		p->start();
}

//*****************************************************************************
void VuPfxSystemInstance::stop(bool bHardKill)
{
	if ( bHardKill )
	{
		mState = STATE_STOPPED;

		for ( VuPfxPatternInstance *p = mPatterns.front(); p; p = p->next() )
			p->destroyParticles();
	}
	else
	{
		mState = STATE_STOPPING;
	}
}

//*****************************************************************************
void VuPfxSystemInstance::tick(float fdt, bool ui)
{
	VU_PROFILE_SIM("PfxTick");

	mAabb.reset();
	mParticleCount = 0;

	if ( mState != STATE_STOPPED )
	{
		mCurrentTime += fdt;

		for ( VuPfxPatternInstance *p = mPatterns.front(); p; p = p->next() )
		{
			p->tick(fdt, ui);
			if ( p->mParticles.size() )
			{
				mAabb.addAabb(p->mAabb);
				mParticleCount += p->mParticles.size();
			}
		}

		if ( mState == STATE_STOPPING && mParticleCount == 0 )
			mState = STATE_STOPPED;

		// if a duration is set, handle self-stopping
		if ( mpParams->mDuration > 0 )
			if ( mState == STATE_ALIVE && mCurrentTime >= mpParams->mDuration )
				mState = STATE_STOPPING;
	}
}

//*****************************************************************************
void VuPfxSystemInstance::draw(const VuGfxDrawParams &params)
{
	if ( VuPfx::IF()->isDrawEnabled() )
		for ( VuPfxPatternInstance *p = mPatterns.front(); p; p = p->next() )
			p->draw(params);
}

//*****************************************************************************
void VuPfxSystemInstance::drawShadow(const VuGfxDrawShadowParams &params)
{
	if ( VuPfx::IF()->isDrawEnabled() )
		for ( VuPfxPatternInstance *p = mPatterns.front(); p; p = p->next() )
			p->drawShadow(params);
}
