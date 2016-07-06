//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Managers/VuTickManager.h"


class VuStaticPfxEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuStaticPfxEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// scripting
	VuRetVal			Start(const VuParams &params = VuParams());
	VuRetVal			Stop(const VuParams &params = VuParams());
	VuRetVal			Kill(const VuParams &params = VuParams());

	void				tickBuild(float fdt);
	void				draw(const VuGfxDrawParams &params);
	void				drawShadow(const VuGfxDrawShadowParams &params);
	void				effectModified();
	void				transformModified();

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	Vu3dDrawComponent		*mp3dDrawComponent;
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	// properties
	std::string			mPfxSystemName;
	bool				mbInitiallyActive;
	float				mPfxScale;
	VuColor				mPfxColor;

	VuPfxSystemInstance	*mpPfxSystemInstance;
};


IMPLEMENT_RTTI(VuStaticPfxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuStaticPfxEntity);


//*****************************************************************************
VuStaticPfxEntity::VuStaticPfxEntity():
	mbInitiallyActive(true),
	mPfxScale(1.0f),
	mPfxColor(255,255,255,255),
	mpPfxSystemInstance(VUNULL)
{
	// properties
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));
	addProperty(new VuStringProperty("Effect Name", mPfxSystemName))->setWatcher(this, &VuStaticPfxEntity::effectModified);
	addProperty(new VuFloatProperty("Pfx Scale", mPfxScale))->setWatcher(this, &VuStaticPfxEntity::effectModified);
	addProperty(new VuColorProperty("Pfx Color", mPfxColor))->setWatcher(this, &VuStaticPfxEntity::effectModified);

	// components
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuStaticPfxEntity::transformModified);
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);

	mp3dDrawComponent->setDrawMethod(this, &VuStaticPfxEntity::draw);
	mp3dDrawComponent->setDrawShadowMethod(this, &VuStaticPfxEntity::drawShadow);

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuStaticPfxEntity, Start);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuStaticPfxEntity, Stop);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuStaticPfxEntity, Kill);
}

//*****************************************************************************
void VuStaticPfxEntity::onGameInitialize()
{
	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuStaticPfxEntity::tickBuild, "Build");

	// create effect
	if ( (mpPfxSystemInstance = VuPfx::IF()->createSystemInstance(mPfxSystemName.c_str())) != VUNULL)
	{
		mpPfxSystemInstance->setScale(mPfxScale);
		mpPfxSystemInstance->setColor(mPfxColor.toVector4());
		mpPfxSystemInstance->setMatrix(mpTransformComponent->getWorldTransform());
	}

	if ( mbInitiallyActive )
		Start();
}

//*****************************************************************************
void VuStaticPfxEntity::onGameRelease()
{
	Kill();

	// release effect
	if ( mpPfxSystemInstance )
	{
		VuPfx::IF()->releaseSystemInstance(mpPfxSystemInstance);
		mpPfxSystemInstance = VUNULL;
	}

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
VuRetVal VuStaticPfxEntity::Start(const VuParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->start();

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuStaticPfxEntity::Stop(const VuParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->stop(false);

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuStaticPfxEntity::Kill(const VuParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->stop(true);

	mp3dDrawComponent->hide();

	return VuRetVal();
}

//*****************************************************************************
void VuStaticPfxEntity::tickBuild(float fdt)
{
	if ( mpPfxSystemInstance )
	{
		mpPfxSystemInstance->tick(fdt, false);

		if ( mpPfxSystemInstance->particleCount() )
		{
			mp3dDrawComponent->show();
			mp3dDrawComponent->updateVisibility(mpPfxSystemInstance->getAabb());
		}
		else
		{
			mp3dDrawComponent->hide();
		}
	}
}

//*****************************************************************************
void VuStaticPfxEntity::draw(const VuGfxDrawParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->draw(params);
}

//*****************************************************************************
void VuStaticPfxEntity::drawShadow(const VuGfxDrawShadowParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->drawShadow(params);
}

//*****************************************************************************
void VuStaticPfxEntity::effectModified()
{
	if ( mpPfxSystemInstance )
	{
		bool bWasAlive = mpPfxSystemInstance->getState() == VuPfxSystemInstance::STATE_ALIVE;

		// re-create effect
		VuPfx::IF()->releaseSystemInstance(mpPfxSystemInstance);
		if ( (mpPfxSystemInstance = VuPfx::IF()->createSystemInstance(mPfxSystemName.c_str())) != VUNULL)
		{
			mpPfxSystemInstance->setMatrix(mpTransformComponent->getWorldTransform());
			mpPfxSystemInstance->setScale(mPfxScale);
			mpPfxSystemInstance->setColor(mPfxColor.toVector4());
			if ( bWasAlive )
				mpPfxSystemInstance->start();
		}
	}
}

//*****************************************************************************
void VuStaticPfxEntity::transformModified()
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->setMatrix(mpTransformComponent->getWorldTransform());
}

//*****************************************************************************
void VuStaticPfxEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform(), false);

	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->setMatrix(mpMotionComponent->getWorldTransform());
}