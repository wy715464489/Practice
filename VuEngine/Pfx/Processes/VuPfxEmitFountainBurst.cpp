//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Fountain Burst Emitter
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxResources.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxQuadParticle.h"
#include "VuEngine/Pfx/Particles/VuPfxGeomParticle.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Properties/VuRotation3dProperty.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Math/VuRand.h"


//*****************************************************************************
// base fountain
//*****************************************************************************
class VuPfxEmitFountainBurst : public VuPfxProcess
{
	DECLARE_RTTI

public:
	VuPfxEmitFountainBurst();

	int			mParticleCount;
	float		mSpawnDelay;
	float		mMinLifespan;
	float		mMaxLifespan;
	VuColor		mMinColor;
	VuColor		mMaxColor;
	float		mMinScale;
	float		mMaxScale;
	VuVector3	mMinLinearVelocity;
	VuVector3	mMaxLinearVelocity;
	VuVector3	mMinPosition;
	VuVector3	mMaxPosition;
	bool		mSpawnAtWaterSurface;
	float		mSpawnDistance;
};

class VuPfxEmitFountainBurstInstance : public VuPfxProcessInstance
{
public:
	VuPfxEmitFountainBurstInstance() : mSpawned(false), mMaxSpawnCountMultiplier(1.0f), mSpawnPerSecondMultiplier(1.0f), mAlphaMultiplier(1.0f) {}

	virtual void		start();
	virtual void		tick(float fdt, bool ui);
	virtual void		onEmit(VuPfxParticle *pParticle) = 0;

	bool				mSpawned;
	float				mMaxSpawnCountMultiplier;
	float				mSpawnPerSecondMultiplier;
	float				mAlphaMultiplier;
};

IMPLEMENT_RTTI(VuPfxEmitFountainBurst, VuPfxProcess);


//*****************************************************************************
VuPfxEmitFountainBurst::VuPfxEmitFountainBurst():
	mParticleCount(10),
	mSpawnDelay(0.0f),
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
	mProperties.add(new VuIntProperty("Particle Count", mParticleCount));
	mProperties.add(new VuFloatProperty("Spawn Delay", mSpawnDelay));
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
void VuPfxEmitFountainBurstInstance::start()
{
	mSpawned = false;
}

//*****************************************************************************
void VuPfxEmitFountainBurstInstance::tick(float fdt, bool ui)
{
	const VuPfxEmitFountainBurst *pParams = static_cast<const VuPfxEmitFountainBurst *>(mpParams);

	if ( !mSpawned && (mpPatternInstance->mpSystemInstance->mCurrentTime >= pParams->mSpawnDelay) )
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
			for ( int i = 0; i < pParams->mParticleCount; i++ )
			{
				// spawn particle
				if ( VuPfxParticle *pParticle = mpPatternInstance->createParticle() )
				{
					pParticle->mAge = 0.0f;
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
				}
			}
		}

		mSpawned = true;
	}
}


//*****************************************************************************
// quad fountain
//*****************************************************************************
class VuPfxEmitQuadFountainBurst : public VuPfxEmitFountainBurst
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxEmitQuadFountainBurst();

	float		mMinRotation;
	float		mMaxRotation;
	float		mMinAngularVelocity;
	float		mMaxAngularVelocity;
	float		mMinWorldScaleZ;
	float		mMaxWorldScaleZ;
	float		mMinDirStretch;
	float		mMaxDirStretch;
	float		mMinTileOffsetU;
	float		mMaxTileOffsetU;
	float		mMinTileOffsetV;
	float		mMaxTileOffsetV;
};

class VuPfxEmitQuadFountainBurstInstance : public VuPfxEmitFountainBurstInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};


IMPLEMENT_RTTI(VuPfxEmitQuadFountainBurst, VuPfxEmitFountainBurst);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitQuadFountainBurst);


//*****************************************************************************
VuPfxEmitQuadFountainBurst::VuPfxEmitQuadFountainBurst():
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
void VuPfxEmitQuadFountainBurstInstance::onEmit(VuPfxParticle *pParticle)
{
	const VuPfxEmitQuadFountainBurst *pParams = static_cast<const VuPfxEmitQuadFountainBurst *>(mpParams);
	VuPfxQuadParticle *pQuadParticle = static_cast<VuPfxQuadParticle *>(pParticle);

	pQuadParticle->mRotation = VuLerp(pParams->mMinRotation, pParams->mMaxRotation, VuRand::global().rand());
	pQuadParticle->mAngularVelocity = VuLerp(pParams->mMinAngularVelocity, pParams->mMaxAngularVelocity, VuRand::global().rand());
	pQuadParticle->mWorldScaleZ = VuLerp(pParams->mMinWorldScaleZ, pParams->mMaxWorldScaleZ, VuRand::global().rand());
	pQuadParticle->mDirStretch = VuLerp(pParams->mMinDirStretch, pParams->mMaxDirStretch, VuRand::global().rand());
	pQuadParticle->mTileOffsetU = VuLerp(pParams->mMinTileOffsetU, pParams->mMaxTileOffsetU, VuRand::global().rand());
	pQuadParticle->mTileOffsetV = VuLerp(pParams->mMinTileOffsetV, pParams->mMaxTileOffsetV, VuRand::global().rand());
}


//*****************************************************************************
// directional quad fountain
//*****************************************************************************

class VuPfxEmitDirectionalQuadFountainBurst : public VuPfxEmitQuadFountainBurst
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS
};

class VuPfxEmitDirectionalQuadFountainBurstInstance : public VuPfxEmitQuadFountainBurstInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};

IMPLEMENT_RTTI(VuPfxEmitDirectionalQuadFountainBurst, VuPfxEmitQuadFountainBurst);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitDirectionalQuadFountainBurst);


//*****************************************************************************
void VuPfxEmitDirectionalQuadFountainBurstInstance::onEmit(VuPfxParticle *pParticle)
{
	VuPfxEmitQuadFountainBurstInstance::onEmit(pParticle);

	VuVector3 vLinVel = pParticle->mLinearVelocity - mpPatternInstance->mpSystemInstance->mLinearVelocity;
	VuVector3 vPos = pParticle->mPosition - mpPatternInstance->getSpawnTransform().getTrans();

	pParticle->mLinearVelocity = mpPatternInstance->mpSystemInstance->mLinearVelocity + vPos.normal()*vLinVel.mag();
}


//*****************************************************************************
// geom fountain
//*****************************************************************************
class VuPfxEmitGeomFountainBurst : public VuPfxEmitFountainBurst
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxEmitGeomFountainBurst();

	VuVector3	mMinRotation;
	VuVector3	mMaxRotation;
	VuVector3	mMinAngularVelocity;
	VuVector3	mMaxAngularVelocity;
};

class VuPfxEmitGeomFountainBurstInstance : public VuPfxEmitFountainBurstInstance
{
public:
	virtual void		onEmit(VuPfxParticle *pParticle);
};


IMPLEMENT_RTTI(VuPfxEmitGeomFountainBurst, VuPfxEmitFountainBurst);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxEmitGeomFountainBurst);


//*****************************************************************************
VuPfxEmitGeomFountainBurst::VuPfxEmitGeomFountainBurst():
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
void VuPfxEmitGeomFountainBurstInstance::onEmit(VuPfxParticle *pParticle)
{
	const VuPfxEmitGeomFountainBurst *pParams = static_cast<const VuPfxEmitGeomFountainBurst *>(mpParams);
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
