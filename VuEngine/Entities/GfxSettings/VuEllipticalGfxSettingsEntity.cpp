//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  EllipticalGfxSettings entity
// 
//*****************************************************************************

#include "VuGfxSettingsEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Managers/VuGfxSettingsManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


class VuEllipticalGfxSettingsEntity : public VuGfxSettingsEntity
{
	DECLARE_RTTI

public:
	VuEllipticalGfxSettingsEntity();

	virtual void	onGameInitialize();
	virtual void	onGameRelease();

private:
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);
	virtual float		getPositionalWeight(const VuVector3 &vPos);

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;

	// properties
	float				mInnerRadius;
};


IMPLEMENT_RTTI(VuEllipticalGfxSettingsEntity, VuGfxSettingsEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuEllipticalGfxSettingsEntity);


//*****************************************************************************
VuEllipticalGfxSettingsEntity::VuEllipticalGfxSettingsEntity():
	mInnerRadius(0.5f)
{
	// properties
	addProperty(new VuPercentageProperty("Inner Radius %", mInnerRadius));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));

	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);
	mp3dLayoutComponent->setDrawMethod(this, &VuEllipticalGfxSettingsEntity::drawLayout);
}

//*****************************************************************************
void VuEllipticalGfxSettingsEntity::onGameInitialize()
{
	VuGfxSettingsEntity::onGameInitialize();

	VuGfxSettingsManager::IF()->add(this);
}

//*****************************************************************************
void VuEllipticalGfxSettingsEntity::onGameRelease()
{
	VuGfxSettingsEntity::onGameRelease();

	VuGfxSettingsManager::IF()->remove(this);
}

//*****************************************************************************
void VuEllipticalGfxSettingsEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.scaleLocal(mpTransformComponent->getWorldScale());

		VuGfxUtil::IF()->drawCylinderLines(VuColor(255,255,0), 0.0f, 1.0f, 16, mat*params.mCamera.getViewProjMatrix());

		mat.scaleLocal(VuVector3(mInnerRadius, mInnerRadius, 1.0f));
		VuGfxUtil::IF()->drawCylinderLines(VuColor(0,255,0), 0.0f, 1.0f, 16, mat*params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
float VuEllipticalGfxSettingsEntity::getPositionalWeight(const VuVector3 &vPos)
{
	const VuMatrix &mat = mpTransformComponent->getWorldTransform();
	const VuVector3 &scale = mpTransformComponent->getWorldScale();

	VuVector3 vDelta = vPos - mat.getTrans();

	VuVector2 vRatio;
	vRatio.mX = VuDot(vDelta, mat.getAxisX())/scale.mX;
	vRatio.mY = VuDot(vDelta, mat.getAxisY())/scale.mY;

	if ( vRatio.magSquared() >= 1.0f )
		return 0.0f;

	float ratio = vRatio.mag();
	if ( ratio < mInnerRadius )
		return 1.0f;

	return (ratio - 1.0f)/(mInnerRadius - 1.0f);
}
