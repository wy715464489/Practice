//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water Ramp entity
// 
//*****************************************************************************

#include "VuWaterSurfaceEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Math/VuMathUtil.h"


class VuWaterRampEntity : public VuWaterSurfaceEntity
{
	DECLARE_RTTI

public:
	VuWaterRampEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

	virtual float		getLocalWaterHeight(float localX, float localY) const;

private:
	virtual void		surfaceModified();
	void				rampModified();
	void				createWaveDesc(VuWaterRampWaveDesc &desc);
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);
	virtual bool		collideLayout(const VuVector3 &v0, VuVector3 &v1);

	float				mSizeZ;
	float				mTransitionRatio;
	float				mFlowSpeed;

	VuWaterRampWave		*mpWave;
};

IMPLEMENT_RTTI(VuWaterRampEntity, VuWaterSurfaceEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuWaterRampEntity);


//*****************************************************************************
VuWaterRampEntity::VuWaterRampEntity():
	mSizeZ(1),
	mTransitionRatio(0.5f),
	mFlowSpeed(10.0f),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Z Size", mSizeZ))								->	setWatcher(this, &VuWaterRampEntity::rampModified);
	addProperty(new VuPercentageProperty("Transition Ratio %", mTransitionRatio))	->	setWatcher(this, &VuWaterRampEntity::rampModified);
	addProperty(new VuFloatProperty("Flow Speed", mFlowSpeed))						->	setWatcher(this, &VuWaterRampEntity::rampModified);
}

//*****************************************************************************
void VuWaterRampEntity::onGameInitialize()
{
	VuWaterSurfaceEntity::onGameInitialize();

	// create wave
	VuWaterRampWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createRampWave(desc);
}

//*****************************************************************************
void VuWaterRampEntity::onGameRelease()
{
	VuWaterSurfaceEntity::onGameRelease();

	// destroy wave
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuWaterRampEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
float VuWaterRampEntity::getLocalWaterHeight(float localX, float localY) const
{
	float height = 0.0f;

	float factor = mTransitionRatio > 0 ? 1.0f/(mTransitionRatio*(2.0f - mTransitionRatio)) : 0;
	float slope = 2.0f*mTransitionRatio*factor;

	float absLocalY = VuAbs(localY);
	if ( absLocalY > (1.0f - mTransitionRatio) )
	{
		if ( localY < 0 )
			height = -1 + (localY + 1)*(localY + 1)*factor;
		else
			height = 1 - (1 - localY)*(1 - localY)*factor;
	}
	else
	{
		height = localY*slope;
	}

	height *= 0.5f*mSizeZ;

	return height;
}

//*****************************************************************************
void VuWaterRampEntity::surfaceModified()
{
	VuWaterSurfaceEntity::surfaceModified();
	rampModified();
}

//*****************************************************************************
void VuWaterRampEntity::rampModified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterRampWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}
}

//*****************************************************************************
void VuWaterRampEntity::createWaveDesc(VuWaterRampWaveDesc &desc)
{
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mSize.mX = (float)mSizeX;
	desc.mSize.mY = (float)mSizeY;
	desc.mSize.mZ = mSizeZ;
	desc.mRotZ = mpTransformComponent->getWorldRotation().mZ;
	desc.mTransitionRatio = mTransitionRatio;
	desc.mFlowSpeed = mFlowSpeed;
}

//*****************************************************************************
void VuWaterRampEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	VuMatrix matMVP = mpTransformComponent->getWorldTransform();
	matMVP.scaleLocal(0.5f*VuVector3((float)mSizeX, (float)mSizeY, mSizeZ));
	matMVP *= params.mCamera.getViewProjMatrix();

	float factor = mTransitionRatio > 0 ? 1.0f/(mTransitionRatio*(2.0f - mTransitionRatio)) : 0;
	float slope = 2.0f*mTransitionRatio*factor;
	float y = 1.0f - mTransitionRatio;
	float z = y*slope;

	VuVector3 verts[4];

	verts[0] = VuVector3(-1, -y, -z);
	verts[1] = VuVector3( 1, -y, -z);
	verts[2] = VuVector3(-1,  y,  z);
	verts[3] = VuVector3( 1,  y,  z);
	VuGfxUtil::IF()->drawTriangleStrip(mShaderDesc.mDiffuseColor, verts, 4, matMVP);

	verts[0] = VuVector3(-1, -1, -1);
	verts[1] = VuVector3( 1, -1, -1);
	verts[2] = VuVector3(-1, -y, -z);
	verts[3] = VuVector3( 1, -y, -z);
	VuGfxUtil::IF()->drawTriangleStrip(mShaderDesc.mDiffuseColor, verts, 4, matMVP);

	verts[0] = VuVector3(-1,  y,  z);
	verts[1] = VuVector3( 1,  y,  z);
	verts[2] = VuVector3(-1,  1,  1);
	verts[3] = VuVector3( 1,  1,  1);
	VuGfxUtil::IF()->drawTriangleStrip(mShaderDesc.mDiffuseColor, verts, 4, matMVP);
}

//*****************************************************************************
bool VuWaterRampEntity::collideLayout(const VuVector3 &v0, VuVector3 &v1)
{
	float factor = mTransitionRatio > 0 ? 1.0f/(mTransitionRatio*(2.0f - mTransitionRatio)) : 0;
	float slope = 2.0f*mTransitionRatio*factor;
	float y = 1.0f - mTransitionRatio;
	float z = y*slope;

	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(0.5f*VuVector3((float)mSizeX, (float)mSizeY, mSizeZ));

	VuVector3 verts[4];
	bool hit = false;

	verts[0] = VuVector3(-1, -y, -z);
	verts[1] = VuVector3( 1, -y, -z);
	verts[2] = VuVector3( 1,  y,  z);
	verts[3] = VuVector3(-1,  y,  z);
	for ( int i = 0; i < 4; i++ )
		verts[i] = mat.transform(verts[i]);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[0], verts[1], verts[2], v0, v1, v1);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[2], verts[3], verts[0], v0, v1, v1);

	verts[0] = VuVector3(-1, -1, -1);
	verts[1] = VuVector3( 1, -1, -1);
	verts[2] = VuVector3( 1, -y, -z);
	verts[3] = VuVector3(-1, -y, -z);
	for ( int i = 0; i < 4; i++ )
		verts[i] = mat.transform(verts[i]);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[0], verts[1], verts[2], v0, v1, v1);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[2], verts[3], verts[0], v0, v1, v1);

	verts[0] = VuVector3(-1,  y,  z);
	verts[1] = VuVector3( 1,  y,  z);
	verts[2] = VuVector3( 1,  1,  1);
	verts[3] = VuVector3(-1,  1,  1);
	for ( int i = 0; i < 4; i++ )
		verts[i] = mat.transform(verts[i]);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[0], verts[1], verts[2], v0, v1, v1);
	hit |= VuMathUtil::triangleLineSegIntersection(verts[2], verts[3], verts[0], v0, v1, v1);

	return hit;
}