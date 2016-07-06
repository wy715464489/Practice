//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Quad Pattern
// 
//*****************************************************************************

#include "VuPfxQuadPattern.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Properties/VuRotation3dProperty.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxResources.h"
#include "VuEngine/Pfx/Particles/VuPfxQuadParticle.h"
#include "VuEngine/Pfx/Shaders/VuPfxQuadShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/Math/VuRand.h"


IMPLEMENT_RTTI(VuPfxQuadPattern, VuPfxPattern);
IMPLEMENT_PFX_PATTERN_REGISTRATION(VuPfxQuadPattern, VuPfxQuadParticle);


// choices for blend mode
static VuStaticIntEnumProperty::Choice sBlendModeChoices[] =
{
	{ "Additive", VuPfxQuadPattern::BM_ADDITIVE},
	{ "Modulate", VuPfxQuadPattern::BM_MODULATE},
	{ VUNULL }
};
VU_COMPILE_TIME_ASSERT(sizeof(sBlendModeChoices)/sizeof(sBlendModeChoices[0]) == VuPfxQuadPattern::BLEND_MODE_COUNT + 1);

// choices for sorting
static VuStaticIntEnumProperty::Choice sSortingChoices[] =
{
	{ "Above Water", VuPfxQuadPattern::SORT_ABOVE_WATER},
	{ "Below Water", VuPfxQuadPattern::SORT_BELOW_WATER},
	{ "UI", VuPfxQuadPattern::SORT_UI},
	{ VUNULL }
};
VU_COMPILE_TIME_ASSERT(sizeof(sSortingChoices)/sizeof(sSortingChoices[0]) == VuPfxQuadPattern::SORTING_COUNT + 1);


//*****************************************************************************
VuPfxQuadPattern::VuPfxQuadPattern():
	mBlendMode(BM_ADDITIVE),
	mSorting(SORT_ABOVE_WATER),
	mClipThreshold(0.0f),
	mNearFadeMin(2.0f),
	mNearFadeMax(4.0f),
	mTileScrollSpeedU(0.0f),
	mTileScrollSpeedV(0.0f),
	mTileScrollLoopTime(1.0f),
	mTileScale(1.0f),
	mMaxStretch(FLT_MAX),
	mFogEnabled(false),
	mCenterOffset(0.0f, 0.0f)
{
	// properties
	mProperties.add(mpTextureAssetProperty = new VuAssetProperty<VuTextureAsset>("Texture Asset", mTextureAssetName));
	mProperties.add(new VuStaticIntEnumProperty("Blend Mode", mBlendMode, sBlendModeChoices));
	mProperties.add(new VuStaticIntEnumProperty("Sorting", mSorting, sSortingChoices));
	mProperties.add(new VuFloatProperty("Clip Threshold", mClipThreshold));
	mProperties.add(new VuFloatProperty("Near Fade Min", mNearFadeMin));
	mProperties.add(new VuFloatProperty("Near Fade Max", mNearFadeMax));
	mProperties.add(mpTileTextureAssetProperty = new VuAssetProperty<VuTextureAsset>("Tile Texture Asset", mTileTextureAssetName));
	mProperties.add(new VuFloatProperty("Tile Scroll Speed U", mTileScrollSpeedU));
	mProperties.add(new VuFloatProperty("Tile Scroll Speed V", mTileScrollSpeedV));
	mProperties.add(new VuFloatProperty("Tile Scroll Loop Time", mTileScrollLoopTime));
	mProperties.add(new VuFloatProperty("Tile Scale", mTileScale));
	mProperties.add(new VuFloatProperty("Max Stretch", mMaxStretch));
	mProperties.add(new VuBoolProperty("Fog Enabled", mFogEnabled));
	mProperties.add(new VuVector2Property("Center Offset", mCenterOffset));
}

//*****************************************************************************
void VuPfxQuadPatternInstance::start()
{
	for ( VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next() )
		p->start();
}

//*****************************************************************************
void VuPfxQuadPatternInstance::tick(float fdt, bool ui)
{
	const VuPfxQuadPattern *pParams = static_cast<const VuPfxQuadPattern *>(mpParams);

	if ( mpSystemInstance->mCurrentTime > mpParams->mStartDelay )
	{
		// handle update
		VuPfxParticle *p = mParticles.front();
		while ( p )
		{
			// basic update
			p->mPosition += p->mLinearVelocity*fdt;
			p->mAge += fdt;

			// quad-specific update
			VuPfxQuadParticle *pq = static_cast<VuPfxQuadParticle *>(p);
			pq->mRotation += pq->mAngularVelocity*fdt;

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
			VuPfxQuadParticle *pq = static_cast<VuPfxQuadParticle *>(p);
			float test = VuSelect(p->mLifespan - p->mAge, 1.0f, -1.0f);
			test = VuSelect(p->mColor.mW, test, -1.0f);
			test = VuSelect(p->mScale, test, -1.0f);
			test = VuSelect(pq->mWorldScaleZ, test, -1.0f);
			if ( test < 0.0f )
			{
				mParticles.remove(p);
				VuPfx::IF()->resources()->freeParticle(p);
			}

			p = pNext;
		}
	}

	// update aabb
	if ( mParticles.size() )
	{
		const VuMatrix &mat = getDrawTransform();
		mAabb.reset();
		float offsetExtent = 0.5f*pParams->mCenterOffset.mag();
		for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
		{
			VuVector3 pos = mat.transform(p->mPosition);
			float fExtent = (0.5f + offsetExtent)*mpSystemInstance->mScale*p->mScale;
			mAabb.mMin.mX = VuMin(mAabb.mMin.mX, pos.mX - fExtent);
			mAabb.mMin.mY = VuMin(mAabb.mMin.mY, pos.mY - fExtent);
			mAabb.mMin.mZ = VuMin(mAabb.mMin.mZ, pos.mZ - fExtent);
			mAabb.mMax.mX = VuMax(mAabb.mMax.mX, pos.mX + fExtent);
			mAabb.mMax.mY = VuMax(mAabb.mMax.mY, pos.mY + fExtent);
			mAabb.mMax.mZ = VuMax(mAabb.mMax.mZ, pos.mZ + fExtent);
		}
	}
	else
	{
		mAabb = VuAabb(VuVector3(0,0,0), VuVector3(0,0,0));
	}
}

//*****************************************************************************
void VuPfxQuadPatternInstance::draw(const VuGfxDrawParams &params)
{
	if ( mParticles.size() )
		VuPfx::IF()->quadShader()->submit(params.mCamera, this);
}


IMPLEMENT_RTTI(VuPfxOrbitQuadPattern, VuPfxQuadPattern);
IMPLEMENT_PFX_PATTERN_REGISTRATION(VuPfxOrbitQuadPattern, VuPfxOrbitQuadParticle);


//*****************************************************************************
VuPfxOrbitQuadPattern::VuPfxOrbitQuadPattern() :
	mOrbitalRadius(1.0f),
	mOrbitalCenter(0.0f, 0.0f, 0.0f),
	mOrbitalRotation(0.0f, 0.0f, 0.0f),
	mOrbitalVelocity(0.0f),
	mMinLifespan(1),
	mMaxLifespan(1),
	mMinColor(255, 255, 255),
	mMaxColor(255, 255, 255),
	mMinScale(1),
	mMaxScale(1),
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
	// properties
	mProperties.add(new VuFloatProperty("Orbital Radius", mOrbitalRadius));
	mProperties.add(new VuVector3Property("Orbital Center", mOrbitalCenter));
	mProperties.add(new VuRotation3dProperty("Orbital Rotation", mOrbitalRotation));
	mProperties.add(new VuAngleProperty("Orbital Velocity", mOrbitalVelocity));
	mProperties.add(new VuFloatProperty("Min Lifespan", mMinLifespan));
	mProperties.add(new VuFloatProperty("Max Lifespan", mMaxLifespan));
	mProperties.add(new VuColorProperty("Min Color", mMinColor));
	mProperties.add(new VuColorProperty("Max Color", mMaxColor));
	mProperties.add(new VuFloatProperty("Min Scale", mMinScale));
	mProperties.add(new VuFloatProperty("Max Scale", mMaxScale));
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
void VuPfxOrbitQuadPatternInstance::tick(float fdt, bool ui)
{
	const VuPfxOrbitQuadPattern *pParams = static_cast<const VuPfxOrbitQuadPattern *>(mpParams);

	if (mpSystemInstance->mCurrentTime > mpParams->mStartDelay)
	{
		// handle particle creation
		if (mParticles.size() == 0 && pParams->mMaxParticleCount)
			createParticles();

		// handle update
		VuMatrix transform;
		transform.setEulerAngles(pParams->mOrbitalRotation);
		transform.setTrans(pParams->mOrbitalCenter);

		VuPfxParticle *p = mParticles.front();
		while (p)
		{
			// basic update
			p->mAge += fdt;

			// quad-specific update
			VuPfxOrbitQuadParticle *poq = static_cast<VuPfxOrbitQuadParticle *>(p);
			poq->mRotation += poq->mAngularVelocity*fdt;

			poq->mOrbitalPosition += pParams->mOrbitalVelocity*fdt;

			float sin, cos;
			VuSinCos(poq->mOrbitalPosition, sin, cos);
			poq->mPosition = transform.transform(VuVector3(pParams->mOrbitalRadius*cos, pParams->mOrbitalRadius*sin, 0.0f));
			poq->mLinearVelocity = VuCross(poq->mPosition - pParams->mOrbitalCenter, -transform.getAxisZ());

			p = p->next();
		}

		for (VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next())
			p->tick(fdt, ui);

		// handle removal
		p = mParticles.front();
		while (p)
		{
			VuPfxParticle *pNext = p->next();

			// removal
			VuPfxQuadParticle *pq = static_cast<VuPfxQuadParticle *>(p);
			float test = VuSelect(p->mLifespan - p->mAge, 1.0f, -1.0f);
			test = VuSelect(p->mColor.mW, test, -1.0f);
			test = VuSelect(p->mScale, test, -1.0f);
			test = VuSelect(pq->mWorldScaleZ, test, -1.0f);
			if (test < 0.0f)
			{
				mParticles.remove(p);
				VuPfx::IF()->resources()->freeParticle(p);
			}

			p = pNext;
		}
	}

	// update aabb
	if (mParticles.size())
	{
		const VuMatrix &mat = getDrawTransform();
		mAabb.reset();
		float offsetExtent = 0.5f*pParams->mCenterOffset.mag();
		for (VuPfxParticle *p = mParticles.front(); p; p = p->next())
		{
			VuVector3 pos = mat.transform(p->mPosition);
			float fExtent = (0.5f + offsetExtent)*mpSystemInstance->mScale*p->mScale;
			mAabb.mMin.mX = VuMin(mAabb.mMin.mX, pos.mX - fExtent);
			mAabb.mMin.mY = VuMin(mAabb.mMin.mY, pos.mY - fExtent);
			mAabb.mMin.mZ = VuMin(mAabb.mMin.mZ, pos.mZ - fExtent);
			mAabb.mMax.mX = VuMax(mAabb.mMax.mX, pos.mX + fExtent);
			mAabb.mMax.mY = VuMax(mAabb.mMax.mY, pos.mY + fExtent);
			mAabb.mMax.mZ = VuMax(mAabb.mMax.mZ, pos.mZ + fExtent);
		}
	}
	else
	{
		mAabb = VuAabb(VuVector3(0, 0, 0), VuVector3(0, 0, 0));
	}
}

//*****************************************************************************
void VuPfxOrbitQuadPatternInstance::createParticles()
{
	const VuPfxOrbitQuadPattern *pParams = static_cast<const VuPfxOrbitQuadPattern *>(mpParams);

	float slice = VU_2PI / pParams->mMaxParticleCount;
	float offset = 0.0f;
	for (int i = 0; i < pParams->mMaxParticleCount; i++)
	{
		// spawn particle
		if (VuPfxParticle *pParticle = createParticle())
		{
			pParticle->mAge = 0.0f;
			pParticle->mLifespan = VuLerp(pParams->mMinLifespan, pParams->mMaxLifespan, VuRand::global().rand());
			pParticle->mColor = VuLerp(pParams->mMinColor.toVector4(), pParams->mMaxColor.toVector4(), VuRand::global().rand());

			pParticle->mScale = VuLerp(pParams->mMinScale, pParams->mMaxScale, VuRand::global().rand());

			pParticle->mPosition.mX = 0.0f;
			pParticle->mPosition.mY = 0.0f;
			pParticle->mPosition.mZ = 0.0f;

			pParticle->mLinearVelocity.mX = 0.0f;
			pParticle->mLinearVelocity.mY = 0.0f;
			pParticle->mLinearVelocity.mZ = 0.0f;

			const VuMatrix &spawnTransform = getSpawnTransform();
			pParticle->mPosition = spawnTransform.transform(pParticle->mPosition);
			pParticle->mLinearVelocity = mpSystemInstance->mLinearVelocity + spawnTransform.transformNormal(pParticle->mLinearVelocity);

			VuPfxOrbitQuadParticle *pOrbitQuadParticle = static_cast<VuPfxOrbitQuadParticle *>(pParticle);

			pOrbitQuadParticle->mRotation = VuLerp(pParams->mMinRotation, pParams->mMaxRotation, VuRand::global().rand());
			pOrbitQuadParticle->mAngularVelocity = VuLerp(pParams->mMinAngularVelocity, pParams->mMaxAngularVelocity, VuRand::global().rand());
			pOrbitQuadParticle->mWorldScaleZ = VuLerp(pParams->mMinWorldScaleZ, pParams->mMaxWorldScaleZ, VuRand::global().rand());
			pOrbitQuadParticle->mDirStretch = VuLerp(pParams->mMinDirStretch, pParams->mMaxDirStretch, VuRand::global().rand());
			pOrbitQuadParticle->mTileOffsetU = VuLerp(pParams->mMinTileOffsetU, pParams->mMaxTileOffsetU, VuRand::global().rand());
			pOrbitQuadParticle->mTileOffsetV = VuLerp(pParams->mMinTileOffsetV, pParams->mMaxTileOffsetV, VuRand::global().rand());
			pOrbitQuadParticle->mOrbitalPosition = offset;
		}

		offset += slice;
	}
}