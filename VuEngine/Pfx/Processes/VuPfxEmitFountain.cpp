//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Fountain Emitter
// 
//*****************************************************************************

#include "VuPfxEmitFountain.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxResources.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxQuadParticle.h"
#include "VuEngine/Pfx/Particles/VuPfxGeomParticle.h"
#include "VuEngine/Pfx/Particles/VuPfxRecursiveParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Properties/VuRotation3dProperty.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Math/VuRand.h"


//*****************************************************************************
// base fountain
//*****************************************************************************

IMPLEMENT_RTTI(VuPfxEmitFountain, VuPfxProcess);


//*****************************************************************************
VuPfxEmitFountain::VuPfxEmitFountain():
	mSpawnPerSecond(10.0f),
	mMaxSpawnCount(0),
	mMinLifespan(1),
	mMaxLifespan(1),
	mMinColor(255,255,255),
	mMaxColor(255,255,255),
	mMinScale(1),
	mMaxScale(1),
	mMinLinearVelocity(0,0,5),
	mMaxLinearVelocity(0,0,5),
	mMinPosition(0,0,0),
	mMaxPosition(0,0,0),
	mSpawnAtWaterSurface(false),
	mSpawnDistance(500.0f)
{
	mProperties.add(new VuFloatProperty("Spawn Per Second", mSpawnPerSecond));
	mProperties.add(new VuIntProperty("Max Spawn Count", mMaxSpawnCount));
	mProperties.add(new VuFloatProperty("Min Lifespan", mMinLifespan));
	mProperties.add(new VuFloatProperty("Max Lifespan", mMaxLifespan));
	mProperties.add(new VuColorProperty("Min Color", mMinColor));
	mProperties.add(new VuColorProperty("Max Color", mMaxColor));
	mProperties.add(new VuFloatProperty("Min Scale", mMinScale));
	mProperties.add(new VuFloatProperty("Max Scale", mMaxScale));
	mProperties.add(new VuVector3Property("Min Position", mMinPosition));
	mProperties.add(new VuVector3Property("Max Position", mMaxPosition));
	mProperties.add(new VuVector3Property("Min Linear Velocity", mMinLinearVelocity));
	mProperties.add(new VuVector3Property("Max Linear Velocity", mMaxLinearVelocity));
	mProperties.add(new VuBoolProperty("Spawn At Water Surface", mSpawnAtWaterSurface));
	mProperties.add(new VuFloatProperty("Spawn Distance", mSpawnDistance));
}

//*****************************************************************************
void VuPfxEmitFountainInstance::start()
{
	mSpawnCount = 0;
	mSpawnAccum = 0;
}

//*****************************************************************************
void VuPfxEmitFountainInstance::tick(float fdt, bool ui)
{
	const VuPfxEmitFountain *pParams = static_cast<const VuPfxEmitFountain *>(mpParams);

	int maxSpawnCount = VuRound(mMaxSpawnCountMultiplier*pParams->mMaxSpawnCount);
	float spawnPerSecond = mSpawnPerSecondMultiplier*pParams->mSpawnPerSecond;

	// check max spawn count (IMPORTANT: check authored max spawn count for 0)
	if ( pParams->mMaxSpawnCount > 0 && mSpawnCount >= maxSpawnCount )
		return;

	mSpawnAccum += spawnPerSecond*fdt;
	while ( mSpawnAccum > 0.0f )
	{
		// check spawn distance
		bool bSpawn = false;
		if ( ui )
		{
			bSpawn = true;
		}
		else
		{
			const VuVector3 &systemPos = mpPatternInstance->mpSystemInstance->mMatrix.getTrans();
			for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
			{
				const VuVector3 &eyePos = VuViewportManager::IF()->getViewport(iViewport).mCamera.getEyePosition();
				if ( VuDistSquared(eyePos, systemPos) < pParams->mSpawnDistance*pParams->mSpawnDistance )
					bSpawn = true;
			}
		}

		if ( bSpawn )
		{
			// spawn particle
			if ( VuPfxParticle *pParticle = mpPatternInstance->createParticle() )
			{
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

				const VuMatrix &spawnTransform = mpPatternInstance->getSpawnTransform();
				pParticle->mPosition = spawnTransform.transform(pParticle->mPosition);
				pParticle->mLinearVelocity = mpPatternInstance->mpSystemInstance->mLinearVelocity + spawnTransform.transformNormal(pParticle->mLinearVelocity);

				onEmit(pParticle);

				if ( pParams->mSpawnAtWaterSurface && VuWater::IF() )
				{
					VuWaterPhysicsVertex vert = VuWater::IF()->getPhysicsVertex(pParticle->mPosition);
					pParticle->mPosition.mZ = vert.mHeight;
				}

				mSpawnCount++;

				// check max spawn count
				if ( maxSpawnCount > 0 && mSpawnCount >= maxSpawnCount )
					return;
			}
		}

		mSpawnAccum -= 1.0f;
	}
}


//*****************************************************************************
// quad fountain
//*****************************************************************************

IMPLEMENT_RTTI(VuPfxEmitQuadFountain, VuPfxEmitFountain);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitQuadFountain);


//*****************************************************************************
VuPfxEmitQuadFountain::VuPfxEmitQuadFountain():
	mMinRotation(0),
	mMaxRotation(0),
	mMinAngularVelocity(0),
	mMaxAngularVelocity(0),
	mMinWorldScaleZ(1),
	mMaxWorldScaleZ(1),
	mMinDirStretch(0),
	mMaxDirStretch(0),
	mMinTileOffsetU(0),
	mMaxTileOffsetU(0),
	mMinTileOffsetV(0),
	mMaxTileOffsetV(0)
{
	mProperties.add(new VuAngleProperty("Min Rotation", mMinRotation));
	mProperties.add(new VuAngleProperty("Max Rotation", mMaxRotation));
	mProperties.add(new VuAngleProperty("Min Angular Velocity", mMinAngularVelocity));
	mProperties.add(new VuAngleProperty("Max Angular Velocity", mMaxAngularVelocity));
	mProperties.add(new VuFloatProperty("Min World Scale Z", mMinWorldScaleZ));
	mProperties.add(new VuFloatProperty("Max World Scale Z", mMaxWorldScaleZ));
	mProperties.add(new VuFloatProperty("Min Directional Stretch", mMinDirStretch));
	mProperties.add(new VuFloatProperty("Max Directional Stretch", mMaxDirStretch));
	mProperties.add(new VuFloatProperty("Min Tile Offset U", mMinTileOffsetU));
	mProperties.add(new VuFloatProperty("Max Tile Offset U", mMaxTileOffsetU));
	mProperties.add(new VuFloatProperty("Min Tile Offset V", mMinTileOffsetV));
	mProperties.add(new VuFloatProperty("Max Tile Offset V", mMaxTileOffsetV));
}

//*****************************************************************************
void VuPfxEmitQuadFountainInstance::onEmit(VuPfxParticle *pParticle)
{
	const VuPfxEmitQuadFountain *pParams = static_cast<const VuPfxEmitQuadFountain *>(mpParams);
	VuPfxQuadParticle *pQuadParticle = static_cast<VuPfxQuadParticle *>(pParticle);

	pQuadParticle->mRotation = VuLerp(pParams->mMinRotation, pParams->mMaxRotation, VuRand::global().rand());
	pQuadParticle->mAngularVelocity = VuLerp(pParams->mMinAngularVelocity, pParams->mMaxAngularVelocity, VuRand::global().rand());
	pQuadParticle->mWorldScaleZ = VuLerp(pParams->mMinWorldScaleZ, pParams->mMaxWorldScaleZ, VuRand::global().rand());
	pQuadParticle->mDirStretch = VuLerp(pParams->mMinDirStretch, pParams->mMaxDirStretch, VuRand::global().rand());
	pQuadParticle->mTileOffsetU = VuLerp(pParams->mMinTileOffsetU, pParams->mMaxTileOffsetU, VuRand::global().rand());
	pQuadParticle->mTileOffsetV = VuLerp(pParams->mMinTileOffsetV, pParams->mMaxTileOffsetV, VuRand::global().rand());
}


//*****************************************************************************
// geom fountain
//*****************************************************************************

IMPLEMENT_RTTI(VuPfxEmitGeomFountain, VuPfxEmitFountain);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitGeomFountain);


//*****************************************************************************
VuPfxEmitGeomFountain::VuPfxEmitGeomFountain():
	mMinRotation(0,0,0),
	mMaxRotation(0,0,0),
	mMinAngularVelocity(0,0,0),
	mMaxAngularVelocity(0,0,0)
{
	mProperties.add(new VuRotation3dProperty("Min Rotation", mMinRotation));
	mProperties.add(new VuRotation3dProperty("Max Rotation", mMaxRotation));
	mProperties.add(new VuRotation3dProperty("Min Angular Velocity", mMinAngularVelocity));
	mProperties.add(new VuRotation3dProperty("Max Angular Velocity", mMaxAngularVelocity));
}

//*****************************************************************************
void VuPfxEmitGeomFountainInstance::onEmit(VuPfxParticle *pParticle)
{
	const VuPfxEmitGeomFountain *pParams = static_cast<const VuPfxEmitGeomFountain *>(mpParams);
	VuPfxGeomParticle *pGeomParticle = static_cast<VuPfxGeomParticle *>(pParticle);

	pGeomParticle->mRotation.mX = VuLerp(pParams->mMinRotation.mX, pParams->mMaxRotation.mX, VuRand::global().rand());
	pGeomParticle->mRotation.mY = VuLerp(pParams->mMinRotation.mY, pParams->mMaxRotation.mY, VuRand::global().rand());
	pGeomParticle->mRotation.mZ = VuLerp(pParams->mMinRotation.mZ, pParams->mMaxRotation.mZ, VuRand::global().rand());
	pGeomParticle->mAngularVelocity.mX = VuLerp(pParams->mMinAngularVelocity.mX, pParams->mMaxAngularVelocity.mX, VuRand::global().rand());
	pGeomParticle->mAngularVelocity.mY = VuLerp(pParams->mMinAngularVelocity.mY, pParams->mMaxAngularVelocity.mY, VuRand::global().rand());
	pGeomParticle->mAngularVelocity.mZ = VuLerp(pParams->mMinAngularVelocity.mZ, pParams->mMaxAngularVelocity.mZ, VuRand::global().rand());

	const VuMatrix &spawnTransform = mpPatternInstance->getSpawnTransform();
	VuMatrix mat;
	mat.setEulerAngles(pGeomParticle->mRotation);
	mat *= spawnTransform;
	pGeomParticle->mRotation = mat.getEulerAngles();
}


//*****************************************************************************
// recursive fountain
//*****************************************************************************

IMPLEMENT_RTTI(VuPfxEmitRecursiveFountain, VuPfxEmitFountain);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitRecursiveFountain);


//*****************************************************************************
VuPfxEmitRecursiveFountain::VuPfxEmitRecursiveFountain():
	mMinRotation(0,0,0),
	mMaxRotation(0,0,0),
	mMinAngularVelocity(0,0,0),
	mMaxAngularVelocity(0,0,0)
{
	mProperties.add(new VuRotation3dProperty("Min Rotation", mMinRotation));
	mProperties.add(new VuRotation3dProperty("Max Rotation", mMaxRotation));
	mProperties.add(new VuRotation3dProperty("Min Angular Velocity", mMinAngularVelocity));
	mProperties.add(new VuRotation3dProperty("Max Angular Velocity", mMaxAngularVelocity));
}

//*****************************************************************************
void VuPfxEmitRecursiveFountainInstance::onEmit(VuPfxParticle *pParticle)
{
	const VuPfxEmitRecursiveFountain *pParams = static_cast<const VuPfxEmitRecursiveFountain *>(mpParams);
	VuPfxRecursiveParticle *pRecursiveParticle = static_cast<VuPfxRecursiveParticle *>(pParticle);

	pRecursiveParticle->mRotation.mX = VuLerp(pParams->mMinRotation.mX, pParams->mMaxRotation.mX, VuRand::global().rand());
	pRecursiveParticle->mRotation.mY = VuLerp(pParams->mMinRotation.mY, pParams->mMaxRotation.mY, VuRand::global().rand());
	pRecursiveParticle->mRotation.mZ = VuLerp(pParams->mMinRotation.mZ, pParams->mMaxRotation.mZ, VuRand::global().rand());
	pRecursiveParticle->mAngularVelocity.mX = VuLerp(pParams->mMinAngularVelocity.mX, pParams->mMaxAngularVelocity.mX, VuRand::global().rand());
	pRecursiveParticle->mAngularVelocity.mY = VuLerp(pParams->mMinAngularVelocity.mY, pParams->mMaxAngularVelocity.mY, VuRand::global().rand());
	pRecursiveParticle->mAngularVelocity.mZ = VuLerp(pParams->mMinAngularVelocity.mZ, pParams->mMaxAngularVelocity.mZ, VuRand::global().rand());

	const VuMatrix &spawnTransform = mpPatternInstance->getSpawnTransform();
	VuMatrix mat;
	mat.setEulerAngles(pRecursiveParticle->mRotation);
	mat *= spawnTransform;
	pRecursiveParticle->mRotation = mat.getEulerAngles();
}


//*****************************************************************************
// directional quad fountain
//*****************************************************************************

IMPLEMENT_RTTI(VuPfxEmitDirectionalQuadFountain, VuPfxEmitQuadFountain);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitDirectionalQuadFountain);


//*****************************************************************************
void VuPfxEmitDirectionalQuadFountainInstance::onEmit(VuPfxParticle *pParticle)
{
	VuPfxEmitQuadFountainInstance::onEmit(pParticle);

	VuVector3 vLinVel = pParticle->mLinearVelocity - mpPatternInstance->mpSystemInstance->mLinearVelocity;
	VuVector3 vPos = pParticle->mPosition - mpPatternInstance->getSpawnTransform().getTrans();

	pParticle->mLinearVelocity = mpPatternInstance->mpSystemInstance->mLinearVelocity + vPos.normal()*vLinVel.mag();
}