//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Ragdoll Splash Emitter
// 
//*****************************************************************************

#include "VuPfxEmitRagdollSplash.h"
#include "VuEngine/Dynamics/VuRagdoll.h"
#include "VuEngine/Dynamics/VuRigidBody.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxQuadParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Math/VuRand.h"


IMPLEMENT_RTTI(VuPfxEmitRagdollSplashQuadFountain, VuPfxEmitQuadFountain);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitRagdollSplashQuadFountain);

#define MAX_BODY_COUNT 32


//*****************************************************************************
VuPfxEmitRagdollSplashQuadFountain::VuPfxEmitRagdollSplashQuadFountain():
	mMinEmitVel(10.0f)
{
	mProperties.add(new VuFloatProperty("Min Emit Velocity", mMinEmitVel));
}

//*****************************************************************************
void VuPfxEmitRagdollSplashQuadFountainInstance::tick(float fdt, bool ui)
{
	const VuPfxEmitRagdollSplashQuadFountain *pParams = static_cast<const VuPfxEmitRagdollSplashQuadFountain *>(mpParams);

	if ( !mpRagdoll )
		return;

	if ( fdt <= FLT_EPSILON )
		return;

	// check spawn distance
	bool bSpawn = false;
	const VuVector3 &systemPos = mpRagdoll->getBody(0).mWorldTransform.getTrans();
	for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
	{
		const VuVector3 &eyePos = VuViewportManager::IF()->getViewport(iViewport).mCamera.getEyePosition();
		if ( VuDistSquared(eyePos, systemPos) < pParams->mSpawnDistance*pParams->mSpawnDistance )
			bSpawn = true;
	}

	if ( !bSpawn )
		return;

	// calculate weights
	VUASSERT(mpRagdoll->getBodyCount() <= MAX_BODY_COUNT, "too many bodies");
	float totalWeight = 0.0f;
	float weights[MAX_BODY_COUNT];
	int bodyCount = mpRagdoll->getBodyCount();
	for ( int i = 0; i < bodyCount; i++ )
	{
		const VuRagdoll::Body &body = mpRagdoll->getBody(i);
		float submergedRatio = body.mSubmergedRatio;
		float linVel = body.mpRigidBody->getLinearVelocity().length();
		weights[i] = linVel > pParams->mMinEmitVel ? submergedRatio*linVel : 0.0f;
		totalWeight += weights[i];
	}

	float spawnPerSecond = totalWeight*pParams->mSpawnPerSecond;

	mSpawnAccum += spawnPerSecond*fdt;
	while ( mSpawnAccum > 0.0f )
	{
		// spawn particle
		if ( VuPfxParticle *pParticle = mpPatternInstance->createParticle() )
		{
			float weightRatio = VuRand::global().rand()*totalWeight;
			int bodyIndex;
			for ( bodyIndex = 0; bodyIndex < bodyCount - 1; bodyIndex++ )
			{
				if ( weightRatio < weights[bodyIndex] )
					break;
				weightRatio -= weights[bodyIndex];
			}
			const VuRagdoll::Body &body = mpRagdoll->getBody(bodyIndex);
			VuVector3 vSpawnPos = body.mWorldTransform.getTrans();
			VuVector3 vSpawnVel = body.mpRigidBody->getVuLinearVelocity();
			vSpawnVel.mZ = 0.0f;

			pParticle->mAge = mSpawnAccum/spawnPerSecond;
			pParticle->mLifespan = VuLerp(pParams->mMinLifespan, pParams->mMaxLifespan, VuRand::global().rand());
			pParticle->mColor = VuLerp(pParams->mMinColor.toVector4(), pParams->mMaxColor.toVector4(), VuRand::global().rand());
			pParticle->mColor.mW *= mAlphaMultiplier;

			pParticle->mScale = VuLerp(pParams->mMinScale, pParams->mMaxScale, VuRand::global().rand());

			pParticle->mPosition.mX = VuLerp(pParams->mMinPosition.mX, pParams->mMaxPosition.mX, VuRand::global().rand());
			pParticle->mPosition.mY = VuLerp(pParams->mMinPosition.mY, pParams->mMaxPosition.mY, VuRand::global().rand());
			pParticle->mPosition.mZ = VuLerp(pParams->mMinPosition.mZ, pParams->mMaxPosition.mZ, VuRand::global().rand());

			pParticle->mLinearVelocity.mX = VuLerp(pParams->mMinLinearVelocity.mX, pParams->mMaxLinearVelocity.mX, VuRand::global().rand());
			pParticle->mLinearVelocity.mY = VuLerp(pParams->mMinLinearVelocity.mY, pParams->mMaxLinearVelocity.mY, VuRand::global().rand());
			pParticle->mLinearVelocity.mZ = VuLerp(pParams->mMinLinearVelocity.mZ, pParams->mMaxLinearVelocity.mZ, VuRand::global().rand());

			pParticle->mPosition = vSpawnPos;
			pParticle->mLinearVelocity = vSpawnVel + pParticle->mLinearVelocity;

			onEmit(pParticle);

			mSpawnCount++;
		}

		mSpawnAccum -= 1.0f;
	}
}
