//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Corona entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Gfx/Corona/VuCorona.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Dev/VuDevConfig.h"


class VuCoronaEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuCoronaEntity();

	virtual void		onPostLoad();
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// scripting
	VuRetVal			Show(const VuParams &params)	{ show(); return VuRetVal(); }
	VuRetVal			Hide(const VuParams &params)	{ hide(); return VuRetVal(); }

	void				tickCorona(float fdt);

	void				show();
	void				hide();

	void				queryRadiusModified();
	void				textureModified();
	void				transformModified();
	void				drawLayout(const Vu3dLayoutDrawParams &params);
	void				draw(const VuGfxDrawParams &params);

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	Vu3dDrawComponent		*mp3dDrawComponent;
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	// properties
	bool				mbInitiallyVisible;
	std::string			mTextureAssetName;
	float				mRotationSpeed;
	VuVector3			mRotationAxis;

	VuCorona			mCorona;
	bool				mbVisible;
	float				mRotation;
};


IMPLEMENT_RTTI(VuCoronaEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCoronaEntity);

//*****************************************************************************
VuCoronaEntity::VuCoronaEntity():
	mbInitiallyVisible(true),
	mRotationSpeed(0.0f),
	mRotationAxis(0,0,1),
	mbVisible(false),
	mRotation(0.0f)
{
	// properties
	addProperty(new VuBoolProperty("Initially Visible", mbInitiallyVisible));
	addProperty(new VuFloatProperty("Draw Distance", mCorona.mDrawDist));
	addProperty(new VuFloatProperty("Fade Distance", mCorona.mFadeDist));
	addProperty(new VuFloatProperty("Query Radius", mCorona.mQueryRadius)) -> setWatcher(this, &VuCoronaEntity::queryRadiusModified);
	addProperty(new VuAngleProperty("Cone Angle", mCorona.mConeAngle));
	addProperty(new VuAngleProperty("Penumbra Angle", mCorona.mPenumbraAngle));
	addProperty(new VuBoolProperty("Enable Back Light",mCorona. mbEnableBackLight));
	addProperty(new VuAssetNameProperty(VuTextureAsset::msRTTI.mstrType, "Texture Name", mTextureAssetName)) -> setWatcher(this, &VuCoronaEntity::textureModified);
	addProperty(new VuBoolProperty("Texture Size Screen Space", mCorona.mbTextureSizeScreenSpace));
	addProperty(new VuFloatProperty("Texture Size", mCorona.mTextureSize));
	addProperty(new VuColorProperty("Texture Color", mCorona.mTextureColor));
	addProperty(new VuAngleProperty("Rotation Offset", mCorona.mRotationOffset));
	addProperty(new VuAngleProperty("Rotation Amount", mCorona.mRotationAmount));
	addProperty(new VuAngleProperty("Rotation Speed", mRotationSpeed));
	addProperty(new VuVector3Property("Rotation Axis", mRotationAxis));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	mpTransformComponent->setWatcher(&VuCoronaEntity::transformModified);
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);
	mp3dDrawComponent->setDrawMethod(this, &VuCoronaEntity::draw);
	mp3dLayoutComponent->setDrawMethod(this, &VuCoronaEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f), VuVector3(0.5f)));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCoronaEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCoronaEntity, Hide);
}

//*****************************************************************************
void VuCoronaEntity::onPostLoad()
{
	queryRadiusModified();
	textureModified();
	transformModified();
}

//*****************************************************************************
void VuCoronaEntity::onGameInitialize()
{
	if ( mbInitiallyVisible )
		show();

	VuTickManager::IF()->registerHandler(this, &VuCoronaEntity::tickCorona, "Corona");

	mRotationAxis.normalize();
}

//*****************************************************************************
void VuCoronaEntity::onGameRelease()
{
	hide();

	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
// As a cpu optimization, these tests can be performed asynchronously
// by the dynamics thread, if another frame of lag is acceptable.
//*****************************************************************************
void VuCoronaEntity::tickCorona(float fdt)
{
	mCorona.updateVisibility(mpTransformComponent->getWorldPosition());

	mRotation += fdt*mRotationSpeed/VU_2PI;
	mRotation -= VuTruncate(mRotation);
}

//*****************************************************************************
void VuCoronaEntity::show()
{
	if ( VuDevConfig::IF()->getParam("DisableCoronas").asBool() )
		return;

	if ( !mbVisible )
	{
		mbVisible = true;
		mp3dDrawComponent->show();
	}
}

//*****************************************************************************
void VuCoronaEntity::hide()
{
	if ( mbVisible )
	{
		mbVisible = false;
		mp3dDrawComponent->hide();
	}
}

//*****************************************************************************
void VuCoronaEntity::queryRadiusModified()
{
	VuVector3 extents(mCorona.mQueryRadius);
	VuAabb aabb(-extents, extents);
	mp3dLayoutComponent->setLocalBounds(VuAabb(-extents, extents));
}

//*****************************************************************************
void VuCoronaEntity::textureModified()
{
	mCorona.setTextureAsset(mTextureAssetName);
}

//*****************************************************************************
void VuCoronaEntity::transformModified()
{
	VuVector3 extents(mCorona.mQueryRadius);
	VuAabb aabb(-extents, extents);
	aabb.mMin += mpTransformComponent->getWorldPosition();
	aabb.mMax += mpTransformComponent->getWorldPosition();
	mp3dDrawComponent->updateVisibility(aabb);
}

//*****************************************************************************
void VuCoronaEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix matMVP = mpTransformComponent->getWorldTransform()*params.mCamera.getViewProjMatrix();

		VuGfxUtil::IF()->drawSphereLines(VuColor(128,128,128), mCorona.mQueryRadius, 8, 8, matMVP);
	}
}

//*****************************************************************************
void VuCoronaEntity::draw(const VuGfxDrawParams &params)
{
	if ( mRotationSpeed == 0.0f )
	{
		mCorona.draw(mpTransformComponent->getWorldTransform(), params);
	}
	else
	{
		VuMatrix xform = mpTransformComponent->getWorldTransform();
		xform.rotateAxisLocal(mRotationAxis, mRotation*VU_2PI);
		mCorona.draw(xform, params);
	}
}

//*****************************************************************************
void VuCoronaEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());
}
