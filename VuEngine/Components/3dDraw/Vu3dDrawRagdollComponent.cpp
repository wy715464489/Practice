//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Vu3dDrawRagdollComponent class
// 
//*****************************************************************************

#include "Vu3dDrawRagdollComponent.h"
#include "VuEngine/Pfx/Processes/VuPfxEmitRagdollSplash.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Assets/VuAnimatedModelAsset.h"
#include "VuEngine/Animation/VuAnimatedSkeleton.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevUtil.h"


IMPLEMENT_RTTI(Vu3dDrawRagdollComponent, Vu3dDrawComponent);


// static variables
static bool sbDebug = false;
static bool sbDebugBones = true;
static bool sbDebugBoneNames = false;
static bool sbDebugRagdoll = true;

#define DEBUG_BONE_SIZE 0.1f


//*****************************************************************************
Vu3dDrawRagdollComponent::Vu3dDrawRagdollComponent(VuEntity *pOwnerEntity):
	Vu3dDrawComponent(pOwnerEntity),
	mDrawDist(FLT_MAX),
	mWaterSimulation(false),
	mpModelInstance(VUNULL),
	mpSplashPfx(VUNULL)
{
	addProperty(mpRagdollTypeProperty = new VuDBEntryProperty("Ragdoll Type", mRagdollType, "RagdollDB"));
	addProperty(new VuFloatProperty("Draw Distance", mDrawDist));
	addProperty(new VuStringProperty("Splash Pfx", mSplashPfx));
	addProperty(new VuBoolProperty("Water Simulation", mWaterSimulation));

	setDrawMethod(this, &Vu3dDrawRagdollComponent::draw);
	setDrawShadowMethod(this, &Vu3dDrawRagdollComponent::drawShadow);

	static VuDevBoolOnce sbOnce;
	if ( sbOnce && VuDevMenu::IF() )
	{
		VuDevMenu::IF()->addBool("Ragdoll/Debug", sbDebug);
		VuDevMenu::IF()->addBool("Ragdoll/DebugDrawBones", sbDebugBones);
		VuDevMenu::IF()->addBool("Ragdoll/DebugDrawBoneNames", sbDebugBoneNames);
		VuDevMenu::IF()->addBool("Ragdoll/DebugRagdoll", sbDebugRagdoll);
	}
}

//*****************************************************************************
Vu3dDrawRagdollComponent::~Vu3dDrawRagdollComponent()
{
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::onGameInitialize()
{
	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &Vu3dDrawRagdollComponent::tickAnim, "Anim");

	// create ragdoll pfx
	if ( (mpSplashPfx = VuPfx::IF()->createSystemInstance(mSplashPfx.c_str())) != VUNULL)
	{
		// set custom pfx data
		VuPfxSystemInstance *pSystemInst = static_cast<VuPfxSystemInstance *>(mpSplashPfx);
		for ( VuPfxPatternInstance *pPatternInst = pSystemInst->mPatterns.front(); pPatternInst; pPatternInst = pPatternInst->next() )
			for ( VuPfxProcessInstance *pProcessInst = pPatternInst->mProcesses.front(); pProcessInst; pProcessInst = pProcessInst->next() )
				if ( pProcessInst->mpParams->isDerivedFrom(VuPfxEmitRagdollSplashQuadFountain::msRTTI) )
					static_cast<VuPfxEmitRagdollSplashQuadFountainInstance *>(pProcessInst)->mpRagdoll = &mRagdoll;
	}
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::onGameRelease()
{
	stopSimulation();

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	if ( mpSplashPfx )
	{
		VuPfx::IF()->releaseSystemInstance(mpSplashPfx);
		mpSplashPfx = VUNULL;
	}
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::setModelInstance(VuAnimatedModelInstance *pModelInstance)
{
	mpModelInstance = pModelInstance;

	// configure ragdoll
	VuRagdoll::Params ragdollParams;
	ragdollParams.mWaterSimulation = mWaterSimulation;
	mRagdoll.configure(mpModelInstance->getSkeleton(), mpRagdollTypeProperty->getEntryData(), getOwnerEntity(), ragdollParams);
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::startSimulation(const VuAnimationTransform *pLocalPose, const VuVector3 &linVel, const VuVector3 &angVel)
{
	mRagdoll.startSimulation(getOwnerEntity()->getTransformComponent()->getWorldTransform(), pLocalPose, linVel, angVel);
	if ( mpSplashPfx )
		mpSplashPfx->start();
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::stopSimulation()
{
	mRagdoll.stopSimulation();
	if ( mpSplashPfx )
		mpSplashPfx->stop();
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::draw(const VuGfxDrawParams &params)
{
	if ( !params.mbDrawReflection )
	{
		float distSquared = (getAabb().getCenter() - params.mEyePos).magSquared();
		if ( distSquared < mDrawDist*mDrawDist )
		{
			const VuMatrix &modelMat = getOwnerEntity()->getTransformComponent()->getWorldTransform();

			if ( sbDebug )
			{
				VuGfxDrawInfoParams infoParams(params.mCamera);

				if ( sbDebugBones )			infoParams.mFlags |= VuGfxDrawInfoParams::BONES;
				if ( sbDebugBoneNames )		infoParams.mFlags |= VuGfxDrawInfoParams::BONE_NAMES;

				infoParams.mBoneSize = DEBUG_BONE_SIZE;

				mpModelInstance->drawInfo(modelMat, infoParams);

				if ( sbDebugRagdoll )
					mRagdoll.drawDebugBodies(params.mCamera, VuColor(255,255,255));
			}
			else
			{
				mpModelInstance->draw(modelMat, params);

				if ( mpSplashPfx )
					mpSplashPfx->draw(params);
			}
		}
	}
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::drawShadow(const VuGfxDrawShadowParams &params)
{
	if ( !(params.mbDrawCollision|params.mbDrawReflection) )
	{
		if ( mpModelInstance->getColor().mA == 255 )
		{
			float distSquared = (getAabb().getCenter() - params.mEyePos).magSquared();
			if ( distSquared < mDrawDist*mDrawDist )
			{
				const VuMatrix &modelMat = getOwnerEntity()->getTransformComponent()->getWorldTransform();
				mpModelInstance->drawShadow(modelMat, params);
			}
		}
	}
}

//*****************************************************************************
void Vu3dDrawRagdollComponent::tickAnim(float fdt)
{
	if ( mRagdoll.isActive() )
	{
		const VuMatrix &modelMat = getOwnerEntity()->getTransformComponent()->getWorldTransform();

		mpModelInstance->setPose(modelMat, &mRagdoll);
		mpModelInstance->finalizePose();

		VuAabb aabb(mpModelInstance->getLocalAabb(), modelMat);

		if ( mpSplashPfx )
		{
			mpSplashPfx->tick(fdt, false);
			aabb.addAabb(mpSplashPfx->getAabb());
		}

		// update visibility
		updateVisibility(aabb);
	}
}
