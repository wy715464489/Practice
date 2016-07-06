//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  RectangularGfxSettings entity
// 
//*****************************************************************************

#include "VuGfxSettingsEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Managers/VuGfxSettingsManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


class VuRectangularGfxSettingsEntity : public VuGfxSettingsEntity
{
	DECLARE_RTTI

public:
	VuRectangularGfxSettingsEntity();

	virtual void	onGameInitialize();
	virtual void	onGameRelease();

private:
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);
	virtual float		getPositionalWeight(const VuVector3 &vPos);

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;

	// properties
	float				mInnerRatioX;
	float				mInnerRatioY;
};


IMPLEMENT_RTTI(VuRectangularGfxSettingsEntity, VuGfxSettingsEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuRectangularGfxSettingsEntity);


//*****************************************************************************
VuRectangularGfxSettingsEntity::VuRectangularGfxSettingsEntity():
	mInnerRatioX(0.5f),
	mInnerRatioY(0.5f)
{
	// properties
	addProperty(new VuPercentageProperty("Inner Radius X %", mInnerRatioX));
	addProperty(new VuPercentageProperty("Inner Radius Y %", mInnerRatioY));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));

	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);
	mp3dLayoutComponent->setDrawMethod(this, &VuRectangularGfxSettingsEntity::drawLayout);
}

//*****************************************************************************
void VuRectangularGfxSettingsEntity::onGameInitialize()
{
	VuGfxSettingsEntity::onGameInitialize();

	VuGfxSettingsManager::IF()->add(this);
}

//*****************************************************************************
void VuRectangularGfxSettingsEntity::onGameRelease()
{
	VuGfxSettingsEntity::onGameRelease();

	VuGfxSettingsManager::IF()->remove(this);
}

//*****************************************************************************
void VuRectangularGfxSettingsEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.scaleLocal(mpTransformComponent->getWorldScale());
		mat.scaleLocal(VuVector3(mInnerRatioX, mInnerRatioY, 0.0f));
		VuGfxUtil::IF()->drawAabbLines(VuColor(0,255,0), mp3dLayoutComponent->getLocalBounds(), mat*params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
float VuRectangularGfxSettingsEntity::getPositionalWeight(const VuVector3 &vPos)
{
	const VuMatrix &mat = mpTransformComponent->getWorldTransform();
	const VuVector3 &scale = mpTransformComponent->getWorldScale();

	VuVector3 vDelta = vPos - mat.getTrans();

	VuVector2 vRatio;
	vRatio.mX = VuAbs(VuDot(vDelta, mat.getAxisX())/scale.mX);
	vRatio.mY = VuAbs(VuDot(vDelta, mat.getAxisY())/scale.mY);

	if ( VuMax(vRatio.mX, vRatio.mY) > 1.0f )
		return 0.0f;

	float weight = 1.0f;

	if ( vRatio.mX > mInnerRatioX )
		weight *= (vRatio.mX - 1.0f)/(mInnerRatioX - 1.0f);

	if ( vRatio.mY > mInnerRatioY )
		weight *= (vRatio.mY - 1.0f)/(mInnerRatioY - 1.0f);

	return weight;
}
