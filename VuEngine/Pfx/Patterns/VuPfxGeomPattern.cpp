//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Geom Pattern
// 
//*****************************************************************************

#include "VuPfxGeomPattern.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxResources.h"
#include "VuEngine/Pfx/Particles/VuPfxGeomParticle.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"


IMPLEMENT_RTTI(VuPfxGeomPattern, VuPfxPattern);
IMPLEMENT_PFX_PATTERN_REGISTRATION(VuPfxGeomPattern, VuPfxGeomParticle);


//*****************************************************************************
VuPfxGeomPattern::VuPfxGeomPattern():
	mRejectionScaleModifier(1.0f),
	mNearFadeMin(0.0f),
	mNearFadeMax(0.0f),
	mFarFadeMin(FLT_MAX),
	mFarFadeMax(FLT_MAX)
{
	// properties
	mProperties.add(new VuAssetNameProperty(VuStaticModelAsset::msRTTI.mstrType, "Model Asset", mModelAssetName))
		->setWatcher(this, &VuPfxGeomPattern::modelAssetModified);
	mProperties.add(new VuFloatProperty("Rejection Scale Modifier", mRejectionScaleModifier))
		->setWatcher(this, &VuPfxGeomPattern::modelAssetModified);
	mProperties.add(new VuFloatProperty("Near Fade Min", mNearFadeMin));
	mProperties.add(new VuFloatProperty("Near Fade Max", mNearFadeMax));
	mProperties.add(new VuFloatProperty("Far Fade Min", mFarFadeMin));
	mProperties.add(new VuFloatProperty("Far Fade Max", mFarFadeMax));
}

//*****************************************************************************
void VuPfxGeomPattern::onLoad()
{
	modelAssetModified();
}

//*****************************************************************************
void VuPfxGeomPattern::modelAssetModified()
{
	mModelInstance.setModelAsset(mModelAssetName);
	mModelInstance.setRejectionScaleModifier(mRejectionScaleModifier);
}

//*****************************************************************************
void VuPfxGeomPatternInstance::start()
{
	for ( VuPfxProcessInstance *p = mProcesses.front(); p; p = p->next() )
		p->start();
}

//*****************************************************************************
void VuPfxGeomPatternInstance::tick(float fdt, bool ui)
{
	const VuPfxGeomPattern *pParams = static_cast<const VuPfxGeomPattern *>(mpParams);

	if ( mpSystemInstance->mCurrentTime > mpParams->mStartDelay )
	{
		// handle update
		VuPfxParticle *p = mParticles.front();
		while ( p )
		{
			// basic update
			p->mPosition += p->mLinearVelocity*fdt;
			p->mAge += fdt;

			// geom-specific update
			VuPfxGeomParticle *pg= static_cast<VuPfxGeomParticle *>(p);
			pg->mRotation += pg->mAngularVelocity*fdt;

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
			float test = VuSelect(p->mLifespan - p->mAge, 1.0f, -1.0f);
			test = VuSelect(p->mColor.mW, test, -1.0f);
			test = VuSelect(p->mScale, test, -1.0f);
			if ( test < 0.0f )
			{
				mParticles.remove(p);
				VuPfx::IF()->resources()->freeParticle(p);
			}

			p = pNext;
		}
	}

	// update aabb using geometry bounding spheres
	if ( mParticles.size() )
	{
		const VuMatrix &mat = getDrawTransform();
		const VuAabb &modelAabb = pParams->mModelInstance.getAabb();
//		VuVector3 vOffset = modelAabb.getCenter();
		float fRadius = modelAabb.getExtents().mag();
		mAabb.reset();
		for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
		{
			VuVector3 pos = mat.transform(p->mPosition);
			float fExtent = mpSystemInstance->mScale*p->mScale*fRadius;
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
void VuPfxGeomPatternInstance::draw(const VuGfxDrawParams &params)
{
	VuPfxGeomPattern *pParams = static_cast<VuPfxGeomPattern *>(mpParams);

	const VuMatrix &drawMat = getDrawTransform();
	float scale = mpSystemInstance->mScale;
	VuVector4 vColor = mpSystemInstance->mColor;

	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxGeomParticle *pg = static_cast<VuPfxGeomParticle *>(p);

		VuMatrix mat = drawMat;
		mat.translateLocal(p->mPosition);

		// handle fading
		float fade = 1.0f;
		float distance = VuDist(drawMat.getTrans(), params.mCamera.getEyePosition());
		fade *= VuLinStep(pParams->mNearFadeMin, pParams->mNearFadeMax, distance);
		fade *= 1.0f - VuLinStep(pParams->mFarFadeMin, pParams->mFarFadeMax, distance);
		if ( fade > FLT_EPSILON )
		{
			mat.rotateXYZLocal(pg->mRotation);
			mat.scaleLocal(VuVector3(scale*p->mScale, scale*p->mScale, scale*p->mScale));

			pParams->mModelInstance.setColor(VuColor(p->mColor*vColor*fade));
			pParams->mModelInstance.draw(mat, params);
		}
	}
}

//*****************************************************************************
void VuPfxGeomPatternInstance::drawShadow(const VuGfxDrawShadowParams &params)
{
	const VuPfxGeomPattern *pParams = static_cast<const VuPfxGeomPattern *>(mpParams);

	VuMatrix drawMat = getDrawTransform();
	float scale = mpSystemInstance->mScale;
	for ( VuPfxParticle *p = mParticles.front(); p; p = p->next() )
	{
		VuPfxGeomParticle *pg = static_cast<VuPfxGeomParticle *>(p);

		VuMatrix mat = drawMat;
		mat.translateLocal(p->mPosition);

		// handle fading
		float fade = 1.0f;
		float distance = VuDist(drawMat.getTrans(), params.mCamera.getEyePosition());
		fade *= VuLinStep(pParams->mNearFadeMin, pParams->mNearFadeMax, distance);
		fade *= 1.0f - VuLinStep(pParams->mFarFadeMin, pParams->mFarFadeMax, distance);
		if ( fade > FLT_EPSILON )
		{
			mat.rotateXYZLocal(pg->mRotation);
			mat.scaleLocal(VuVector3(scale*p->mScale, scale*p->mScale, scale*p->mScale));

			pParams->mModelInstance.drawShadow(mat, params);
		}
	}
}
