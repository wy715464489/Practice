//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Trail Pattern
// 
//*****************************************************************************

#include "VuPfxTrailPattern.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxResources.h"
#include "VuEngine/Pfx/Particles/VuPfxTrailParticle.h"
#include "VuEngine/Pfx/Shaders/VuPfxTrailShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Managers/VuViewportManager.h"


IMPLEMENT_RTTI(VuPfxTrailPattern, VuPfxPattern);
IMPLEMENT_PFX_PATTERN_REGISTRATION(VuPfxTrailPattern, VuPfxTrailParticle);


// choices for trail type
static VuStaticIntEnumProperty::Choice sTrailTypeChoices[] =
{
	{ "2D Trail", VuPfxTrailPattern::TT_2D_TRAIL},
	{ "3D Ribbon", VuPfxTrailPattern::TT_3D_RIBBON},
	{ VUNULL }
};
VU_COMPILE_TIME_ASSERT(sizeof(sTrailTypeChoices)/sizeof(sTrailTypeChoices[0]) == VuPfxTrailPattern::TRAIL_TYPE_COUNT + 1);

// choices for blend mode
static VuStaticIntEnumProperty::Choice sBlendModeChoices[] =
{
	{ "Additive", VuPfxTrailPattern::BM_ADDITIVE},
	{ "Modulate", VuPfxTrailPattern::BM_MODULATE},
	{ VUNULL }
};
VU_COMPILE_TIME_ASSERT(sizeof(sBlendModeChoices)/sizeof(sBlendModeChoices[0]) == VuPfxTrailPattern::BLEND_MODE_COUNT + 1);

// choices for sorting
static VuStaticIntEnumProperty::Choice sSortingChoices[] =
{
	{ "Above Water", VuPfxTrailPattern::SORT_ABOVE_WATER},
	{ "Below Water", VuPfxTrailPattern::SORT_BELOW_WATER},
	{ "UI", VuPfxTrailPattern::SORT_UI},
	{ VUNULL }
};
VU_COMPILE_TIME_ASSERT(sizeof(sSortingChoices)/sizeof(sSortingChoices[0]) == VuPfxTrailPattern::SORTING_COUNT + 1);


//*****************************************************************************
VuPfxTrailPattern::VuPfxTrailPattern():
	mTrailType(TT_2D_TRAIL),
	mLifespan(1),
	mFadeInTime(0),
	mFadeOutStartTime(0),
	mColor(255,255,255),
	mWidth(1),
	mLinearVelocity(0,0,0),
	mSpawnDistance(500.0f),
	mTexCoordRate(1.0f),
	mBlendMode(BM_ADDITIVE),
	mSorting(SORT_ABOVE_WATER)
{
	// properties
	mProperties.add(new VuStaticIntEnumProperty("Trail Type", mTrailType, sTrailTypeChoices));
	mProperties.add(new VuFloatProperty("Lifespan", mLifespan));
	mProperties.add(new VuFloatProperty("Fade In Time", mFadeInTime));
	mProperties.add(new VuFloatProperty("Fade Out Start Time", mFadeOutStartTime));
	mProperties.add(new VuColorProperty("Color", mColor));
	mProperties.add(new VuFloatProperty("Width", mWidth));
	mProperties.add(new VuVector3Property("Linear Velocity", mLinearVelocity));
	mProperties.add(new VuFloatProperty("Spawn Distance", mSpawnDistance));
	mProperties.add(mpTextureAssetProperty = new VuAssetProperty<VuTextureAsset>("Texture Asset", mTextureAssetName));
	mProperties.add(new VuFloatProperty("Tex Coord Rate", mTexCoordRate));
	mProperties.add(new VuStaticIntEnumProperty("Blend Mode", mBlendMode, sBlendModeChoices));
	mProperties.add(new VuStaticIntEnumProperty("Sorting", mSorting, sSortingChoices));
}

//*****************************************************************************
void VuPfxTrailPatternInstance::start()
{
	for ( VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next() )
		p->start();

	mSpawnAccum = 0.0f;
	mAliveTime = 0.0f;
	mpRootParticle = VUNULL;
}

//*****************************************************************************
void VuPfxTrailPatternInstance::tick(float fdt, bool ui)
{
	const VuPfxTrailPattern *pParams = static_cast<const VuPfxTrailPattern *>(mpParams);

	if ( mpSystemInstance->mCurrentTime > mpParams->mStartDelay )
	{
		// calculate fade params
		float fAlpha = pParams->mColor.mA/255.0f;
		float fFadeTime = VuMax(pParams->mLifespan - pParams->mFadeOutStartTime, 0.0f);

		// handle emission
		if ( mpSystemInstance->mState == VuPfxSystemInstance::STATE_ALIVE )
		{
			float fSpawnPerSecond = pParams->mMaxParticleCount/pParams->mLifespan;
			mSpawnAccum += fSpawnPerSecond*fdt;
			if ( mSpawnAccum > 0.0f )
			{
				mpRootParticle = VUNULL;

				// check spawn distance
				bool bSpawn = false;
				if ( ui )
				{
					bSpawn = true;
				}
				else
				{
					const VuVector3 &systemPos = mpSystemInstance->mMatrix.getTrans();
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
					if ( VuPfxParticle *pParticle = VuPfx::IF()->resources()->allocateParticle(mpParams) )
					{
						mParticles.push_front(pParticle);
						mpRootParticle = static_cast<VuPfxTrailParticle *>(pParticle);
					}
				}

				if ( mParticles.size() < 2 )
				{
					mSpawnAccum = 0.0f;
				}
				else
				{
					while ( mSpawnAccum > 0.0f )
						mSpawnAccum -= 1.0f;
				}
			}
		}

		// handle update
		{
			for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
			{
				//VuPfxTrailParticle *pt = static_cast<VuPfxTrailParticle *>(p);

				// basic update
				p->mPosition += p->mLinearVelocity*fdt;
				p->mAge += fdt;

				// trail-specific update
				float fFadeIn = VuClamp(p->mAge/pParams->mFadeInTime, 0.0f, 1.0f);
				float fFadeOut = VuClamp((pParams->mLifespan - p->mAge)/fFadeTime, 0.0f, 1.0f);
				p->mColor.mW = fAlpha*VuMin(fFadeIn, fFadeOut);
			}

			for ( VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next() )
				p->tick(fdt, ui);
		}

		// update root
		if ( mpRootParticle )
		{
			if ( mpSystemInstance->mState == VuPfxSystemInstance::STATE_ALIVE )
			{
				const VuMatrix &spawnTransform = getSpawnTransform();

				mpRootParticle->mPosition = spawnTransform.getTrans();
				mpRootParticle->mLinearVelocity = spawnTransform.transformNormal(pParams->mLinearVelocity);
				mpRootParticle->mColor = pParams->mColor.toVector4();
				mpRootParticle->mScale = pParams->mWidth;
				mpRootParticle->mLifespan = pParams->mLifespan;

				mpRootParticle->mAge = VuMax(pParams->mLifespan - mAliveTime, 0.0f);

				float fFadeIn = VuClamp(mpRootParticle->mAge/pParams->mFadeInTime, 0.0f, 1.0f);
				float fFadeOut = VuClamp((pParams->mLifespan - mpRootParticle->mAge)/fFadeTime, 0.0f, 1.0f);
				mpRootParticle->mColor.mW = fAlpha*VuMin(fFadeIn, fFadeOut);
				mpRootParticle->mColor.mW = VuMax(mpRootParticle->mColor.mW, FLT_EPSILON);

				mpRootParticle->mAxis = spawnTransform.getAxisX();
				mpRootParticle->mTexCoord = -pParams->mTexCoordRate*mAliveTime;
			}
			else
			{
				mpRootParticle = VUNULL;
			}
		}

		// remove last particle if needed
		if ( mParticles.size() >= 2 )
		{
			VuPfxParticle *p = mParticles.back();
			if ( p->prev()->mColor.mW <= 0.0f )
			{
				mParticles.remove(p);
				VuPfx::IF()->resources()->freeParticle(p);
			}
		}
		else if ( mpSystemInstance->mState != VuPfxSystemInstance::STATE_ALIVE )
		{
			if ( mParticles.size() == 1 )
			{
				VuPfx::IF()->resources()->freeParticle(mParticles.front());
				mParticles.clear();
				mpRootParticle = VUNULL;
			}
		}

		// update alive time
		mAliveTime += fdt;
		if ( mpSystemInstance->mState != VuPfxSystemInstance::STATE_ALIVE )
			mAliveTime = 0.0f;
	}

	// update aabb
	if ( mParticles.size() )
	{
		const VuMatrix &mat = getDrawTransform();
		mAabb.reset();
		for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
		{
			VuVector3 pos = mat.transform(p->mPosition);
			float fExtent = 0.5f*p->mScale;
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
void VuPfxTrailPatternInstance::draw(const VuGfxDrawParams &params)
{
	if ( mParticles.size() )
		VuPfx::IF()->trailShader()->submit(params.mCamera, this);
}
