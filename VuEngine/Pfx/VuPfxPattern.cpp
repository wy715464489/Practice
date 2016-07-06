//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Pattern
// 
//*****************************************************************************

#include "VuPfxPattern.h"
#include "VuPfx.h"
#include "VuPfxSystem.h"
#include "VuPfxResources.h"
#include "VuPfxRegistry.h"
#include "VuPfxProcess.h"
#include "VuPfxParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Json/VuJsonContainer.h"


IMPLEMENT_RTTI(VuPfxPattern, VuPfxNode);


// choices for space
static VuStaticIntEnumProperty::Choice sSpaceChoices[] =
{
	{ "World", VuPfxPattern::WORLD_SPACE},
	{ "Object", VuPfxPattern::OBJECT_SPACE},
	{ VUNULL }
};

//*****************************************************************************
VuPfxPattern::VuPfxPattern():
	mMaxParticleCount(0),
	mSpace(WORLD_SPACE),
	mStartDelay(0.0f)
{
	// properties
	mProperties.add(new VuIntProperty("Max Particle Count", mMaxParticleCount));
	mProperties.add(new VuStaticIntEnumProperty("Space", mSpace, sSpaceChoices));
	mProperties.add(new VuFloatProperty("Start Delay", mStartDelay));
}

//*****************************************************************************
VuPfxPatternInstance::VuPfxPatternInstance():
	mAabb(VuVector3(0,0,0), VuVector3(0,0,0))
{
}

//*****************************************************************************
bool VuPfxPatternInstance::create()
{
	// create Processes
	for ( VuPfxPattern::ChildNodes::iterator iter = mpParams->mChildNodes.begin(); iter != mpParams->mChildNodes.end(); iter++ )
	{
		VuPfxProcessInstance *pProcessInstance = VuPfx::IF()->resources()->allocateProcess(static_cast<VuPfxProcess *>(iter->second));
		if ( pProcessInstance == VUNULL )
			return false;

		pProcessInstance->mpPatternInstance = this;

		mProcesses.push_back(pProcessInstance);
	}

	return true;
}

//*****************************************************************************
void VuPfxPatternInstance::destroy()
{
	while ( VuPfxProcessInstance *pProcessInstance = mProcesses.pop_back() )
		VuPfx::IF()->resources()->freeProcess(pProcessInstance);

	while ( VuPfxParticle *pParticle = mParticles.pop_back() )
		VuPfx::IF()->resources()->freeParticle(pParticle);
}

//*****************************************************************************
void VuPfxPatternInstance::destroyParticles()
{
	while ( VuPfxParticle *pParticle = mParticles.pop_back() )
		VuPfx::IF()->resources()->freeParticle(pParticle);
}

//*****************************************************************************
VuPfxParticle *VuPfxPatternInstance::createParticle()
{
	// make sure system is alive
	if ( mpSystemInstance->mState != VuPfxSystemInstance::STATE_ALIVE )
		return VUNULL;

	// check for too many particles in pattern
	if ( mpParams->mMaxParticleCount && mParticles.size() >= mpParams->mMaxParticleCount )
		return VUNULL;

	if ( VuPfxParticle *pParticle = VuPfx::IF()->resources()->allocateParticle(mpParams) )
	{
		mParticles.push_front(pParticle);
		return pParticle;
	}

	return VUNULL;
}

//*****************************************************************************
const VuMatrix &VuPfxPatternInstance::getSpawnTransform() const
{
	if ( mpParams->mSpace == VuPfxPattern::OBJECT_SPACE )
		return VuMatrix::identity();

	return mpSystemInstance->mMatrix;
}

//*****************************************************************************
const VuMatrix &VuPfxPatternInstance::getDrawTransform() const
{
	if ( mpParams->mSpace == VuPfxPattern::OBJECT_SPACE )
		return mpSystemInstance->mMatrix;

	return VuMatrix::identity();
}
