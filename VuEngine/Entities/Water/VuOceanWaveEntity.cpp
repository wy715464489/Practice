//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ocean wave entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Water/VuWater.h"


//*****************************************************************************
// Base entity for ocean waves.  Deals with properties common to all ocean
// wave types.
//*****************************************************************************
class VuBaseOceanWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuBaseOceanWaveEntity();

	virtual void			onPostLoad() { modified(); }

protected:
	virtual void					modified() = 0;
	virtual VuWaterBaseOceanWave	*baseWave() = 0;
	void							createWaveDesc(VuWaterBaseOceanWaveDesc &desc);

	// components
	VuScriptComponent		*mpScriptComponent;

	// properties
	int						mComplexityEnum;
	float					mBinSize;
	float					mWaveDirection;
	float					mGravity;
	float					mWindSpeed;
	float					mDirectionalPower;
	float					mSuppressionWaveLength;
	float					mHeightMultiplier;
};

//*****************************************************************************
// Infinite ocean wave.  This ocean wave is not bounded.
//*****************************************************************************
class VuInfiniteOceanWaveEntity : public VuBaseOceanWaveEntity
{
	DECLARE_RTTI

public:
	VuInfiniteOceanWaveEntity();

	virtual void				onGameInitialize();
	virtual void				onGameRelease();

private:
	virtual void					modified();
	virtual VuWaterBaseOceanWave	*baseWave() { return mpWave; }


	VuWaterInfiniteOceanWave	*mpWave;
};

//*****************************************************************************
// Rectangular ocean wave.  This ocean wave is bounded by a rectangle.
//*****************************************************************************
class VuRectangularOceanWaveEntity : public VuBaseOceanWaveEntity
{
	DECLARE_RTTI

public:
	VuRectangularOceanWaveEntity();

	virtual void				onGameInitialize();
	virtual void				onGameRelease();

private:
	virtual void					modified();
	virtual VuWaterBaseOceanWave	*baseWave() { return mpWave; }
	void							createWaveDesc(VuWaterRectangularOceanWaveDesc &desc);
	void							drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent			*mp3dLayoutComponent;

	float						mDecayRatioX;
	float						mDecayRatioY;

	VuWaterRectangularOceanWave	*mpWave;
};

//*****************************************************************************
// Circular ocean wave.  This ocean wave is bounded by a circle.
//*****************************************************************************
class VuCircularOceanWaveEntity : public VuBaseOceanWaveEntity
{
	DECLARE_RTTI

public:
	VuCircularOceanWaveEntity();

	virtual void				onGameInitialize();
	virtual void				onGameRelease();

private:
	virtual void					modified();
	virtual VuWaterBaseOceanWave	*baseWave() { return mpWave; }
	void							createWaveDesc(VuWaterCircularOceanWaveDesc &desc);
	void							drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent			*mp3dLayoutComponent;

	float						mRadius;
	float						mDecayRatio;

	VuWaterCircularOceanWave	*mpWave;
};


IMPLEMENT_RTTI(VuBaseOceanWaveEntity, VuEntity);
IMPLEMENT_RTTI(VuInfiniteOceanWaveEntity, VuBaseOceanWaveEntity);
IMPLEMENT_RTTI(VuRectangularOceanWaveEntity, VuBaseOceanWaveEntity);
IMPLEMENT_RTTI(VuCircularOceanWaveEntity, VuBaseOceanWaveEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuInfiniteOceanWaveEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuRectangularOceanWaveEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCircularOceanWaveEntity);


// choices for complexity
static VuStaticIntEnumProperty::Choice sComplexityChoices[] =
{
	{ "4", 4},
	{ "5", 5},
	{ "6", 6},
	{ "7", 7},
	{ "8", 8},
	{ VUNULL }
};

//*****************************************************************************
VuBaseOceanWaveEntity::VuBaseOceanWaveEntity():
	mComplexityEnum(5),
	mBinSize(100.0f),
	mWaveDirection(0),
	mGravity(9.806f),
	mWindSpeed(5.0f),
	mDirectionalPower(3.0f),
	mSuppressionWaveLength(0.5f),
	mHeightMultiplier(0.1f)
{
	// properties
	addProperty(new VuStaticIntEnumProperty("Complexity", mComplexityEnum, sComplexityChoices))	->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuFloatProperty("Bin Siz", mBinSize))										->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuAngleProperty("Wave Direction", mWaveDirection))							->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuFloatProperty("Gravity", mGravity))										->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuFloatProperty("Wind Speed", mWindSpeed))									->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuFloatProperty("Directional Power", mDirectionalPower))					->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuFloatProperty("Suppression Wave Length", mSuppressionWaveLength))			->	setWatcher(this, &VuBaseOceanWaveEntity::modified);
	addProperty(new VuFloatProperty("Height Multiplier", mHeightMultiplier))					->	setWatcher(this, &VuBaseOceanWaveEntity::modified);

	// component
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
}

//*****************************************************************************
void VuBaseOceanWaveEntity::createWaveDesc(VuWaterBaseOceanWaveDesc &desc)
{
	desc.mComplexity = mComplexityEnum;
	desc.mBinSize = mBinSize;
	desc.mWaveDirection = mWaveDirection;
	desc.mGravity = mGravity;
	desc.mWindSpeed = mWindSpeed;
	desc.mDirectionalPower = mDirectionalPower;
	desc.mSuppressionWaveLength = mSuppressionWaveLength;
	desc.mHeightMultiplier = mHeightMultiplier;
}

//*****************************************************************************
VuInfiniteOceanWaveEntity::VuInfiniteOceanWaveEntity():
	mpWave(VUNULL)
{
}

//*****************************************************************************
void VuInfiniteOceanWaveEntity::onGameInitialize()
{
	// create wave
	VuWaterInfiniteOceanWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createInfiniteOceanWave(desc);
}

//*****************************************************************************
void VuInfiniteOceanWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuInfiniteOceanWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuInfiniteOceanWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterInfiniteOceanWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}
}

//*****************************************************************************
VuRectangularOceanWaveEntity::VuRectangularOceanWaveEntity():
	mDecayRatioX(0.9f),
	mDecayRatioY(0.9f),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuPercentageProperty("Decay Ratio X %", mDecayRatioX))	->	setWatcher(this, &VuRectangularOceanWaveEntity::modified);
	addProperty(new VuPercentageProperty("Decay Ratio Y %", mDecayRatioY))	->	setWatcher(this, &VuRectangularOceanWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuRectangularOceanWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuRectangularOceanWaveEntity::modified);

	// limit manipulation to translation and scale in x-y
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);
}

//*****************************************************************************
void VuRectangularOceanWaveEntity::onGameInitialize()
{
	// create wave
	VuWaterRectangularOceanWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createRectangularOceanWave(desc);
}

//*****************************************************************************
void VuRectangularOceanWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuRectangularOceanWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuRectangularOceanWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterRectangularOceanWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f, -0.5f, 0), VuVector3(0.5f, 0.5f, 1)));
}

//*****************************************************************************
void VuRectangularOceanWaveEntity::createWaveDesc(VuWaterRectangularOceanWaveDesc &desc)
{
	VuBaseOceanWaveEntity::createWaveDesc(desc);
	desc.mPosition = mpTransformComponent->getWorldPosition();
	desc.mSizeX = mpTransformComponent->getWorldScale().mX;
	desc.mSizeY = mpTransformComponent->getWorldScale().mY;
	desc.mDecayRatioX = mDecayRatioX;
	desc.mDecayRatioY = mDecayRatioY;
}

//*****************************************************************************
void VuRectangularOceanWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuVector3 vScale = mpTransformComponent->getWorldScale();
		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP.scaleLocal(VuVector3(0.5f*vScale.mX, 0.5f*vScale.mY, 1));
		matMVP *= params.mCamera.getViewProjMatrix();

		// draw decay ratios
		{
			VuColor col(255,64,64);
			float rx = mDecayRatioX;
			float ry = mDecayRatioY;
			pGfxUtil->drawLine3d(col, VuVector3(-rx, -ry, 1), VuVector3( rx, -ry, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3(-rx,  ry, 1), VuVector3( rx,  ry, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3(-rx, -ry, 1), VuVector3(-rx,  ry, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3( rx, -ry, 1), VuVector3( rx,  ry, 1), matMVP);
		}
	}
}

//*****************************************************************************
VuCircularOceanWaveEntity::VuCircularOceanWaveEntity():
	mRadius(100.0f),
	mDecayRatio(0.9f),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Radius", mRadius))					->	setWatcher(this, &VuCircularOceanWaveEntity::modified);
	addProperty(new VuPercentageProperty("Decay Ratio %", mDecayRatio))	->	setWatcher(this, &VuCircularOceanWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuCircularOceanWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuCircularOceanWaveEntity::modified);

	// limit manipulation to translation
	mpTransformComponent->setMask(VuTransformComponent::TRANS);


	// update 3d layout bounds
	modified();
}

//*****************************************************************************
void VuCircularOceanWaveEntity::onGameInitialize()
{
	// create
	VuWaterCircularOceanWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createCircularOceanWave(desc);
}

//*****************************************************************************
void VuCircularOceanWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuCircularOceanWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuCircularOceanWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterCircularOceanWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-mRadius, -mRadius, 0), VuVector3(mRadius, mRadius, 1)));
}

//*****************************************************************************
void VuCircularOceanWaveEntity::createWaveDesc(VuWaterCircularOceanWaveDesc &desc)
{
	VuBaseOceanWaveEntity::createWaveDesc(desc);
	desc.mPosition = mpTransformComponent->getWorldPosition();
	desc.mRadius = mRadius;
	desc.mDecayRatio = mDecayRatio;
}

//*****************************************************************************
void VuCircularOceanWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP *= params.mCamera.getViewProjMatrix();

		// draw inner & outer radii
		pGfxUtil->drawCylinderLines(VuColor(128,128,128), 1, mRadius, 32, matMVP);
		pGfxUtil->drawCylinderLines(VuColor(255,64,64), 1, mRadius*mDecayRatio, 32, matMVP);
	}
}
