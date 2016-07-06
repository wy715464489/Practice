//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Pfx entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/UI/VuUIPropertyUtil.h"


class VuUIPfxEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUIPfxEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

protected:
	// event handlers
	void				OnUIDraw(const VuParams &params);

	// scripting
	VuRetVal			Start(const VuParams &params);
	VuRetVal			Stop(const VuParams &params);
	VuRetVal			Kill(const VuParams &params);

	void				drawLayout(bool bSelected);

	void				tickBuild(float fdt);

	void				calcTransform(VuMatrix &transform);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string				mPfxName;
	VuVector2				mPfxPos;
	float					mPfxDist;
	bool					mInitiallyActive;
	bool					mUseRealTime;
	VuUIAnchorProperties	mAnchor;

	VuPfxSystemInstance	*mpPfxSystemInstance;
};


IMPLEMENT_RTTI(VuUIPfxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIPfxEntity);


//*****************************************************************************
VuUIPfxEntity::VuUIPfxEntity():
	mPfxPos(0,0),
	mPfxDist(10.0f),
	mInitiallyActive(false),
	mUseRealTime(false),
	mpPfxSystemInstance(VUNULL)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
	addComponent(new Vu2dLayoutComponent(this, &VuUIPfxEntity::drawLayout));

	// properties
	addProperty(new VuStringProperty("Pfx Name", mPfxName));
	addProperty(new VuVector2Property("Pfx Pos", mPfxPos));
	addProperty(new VuFloatProperty("Pfx Dist", mPfxDist));
	addProperty(new VuBoolProperty("Initially Active", mInitiallyActive));
	addProperty(new VuBoolProperty("Use Real Time", mUseRealTime));
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPfxEntity, Start);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPfxEntity, Stop);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPfxEntity, Kill);

	// event handlers
	REG_EVENT_HANDLER(VuUIPfxEntity, OnUIDraw);
}

//*****************************************************************************
void VuUIPfxEntity::onGameInitialize()
{
	VuTickManager::IF()->registerHandler(this, &VuUIPfxEntity::tickBuild, "Build");

	mpPfxSystemInstance = VuPfx::IF()->createSystemInstance(mPfxName.c_str());

	if ( mInitiallyActive )
		Start(VuParams());
}

//*****************************************************************************
void VuUIPfxEntity::onGameRelease()
{
	if ( mpPfxSystemInstance )
	{
		VuPfx::IF()->releaseSystemInstance(mpPfxSystemInstance);
		mpPfxSystemInstance = VUNULL;
	}

	VuTickManager::IF()->unregisterHandler(this, "Build");
}

//*****************************************************************************
void VuUIPfxEntity::OnUIDraw(const VuParams &params)
{
	float alpha = 1.0f;

	if ( mpPfxSystemInstance )
	{
		const VuCamera &camera = VuViewportManager::IF()->getUiCamera();
		mpPfxSystemInstance->draw(VuGfxDrawParams(camera));
	}
}

//*****************************************************************************
VuRetVal VuUIPfxEntity::Start(const VuParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->start();

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuUIPfxEntity::Stop(const VuParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->stop();

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuUIPfxEntity::Kill(const VuParams &params)
{
	if ( mpPfxSystemInstance )
		mpPfxSystemInstance->stop(true);

	return VuRetVal();
}

//*****************************************************************************
void VuUIPfxEntity::drawLayout(bool bSelected)
{
	// draw rect
	if ( bSelected )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);

		VuGfxUtil::IF()->drawLine2d(uiDrawParams.mDepth, VuColor(255,255,255), uiDrawParams.transform(mPfxPos - VuVector2(10,0)), uiDrawParams.transform(mPfxPos + VuVector2(10,0)));
		VuGfxUtil::IF()->drawLine2d(uiDrawParams.mDepth, VuColor(255,255,255), uiDrawParams.transform(mPfxPos - VuVector2(0,10)), uiDrawParams.transform(mPfxPos + VuVector2(0,10)));
	}
}

//*****************************************************************************
void VuUIPfxEntity::tickBuild(float fdt)
{
	if ( mUseRealTime )
		fdt = VuTickManager::IF()->getRealDeltaTime();

	if ( mpPfxSystemInstance )
	{
		VuMatrix transform;
		calcTransform(transform);
		mpPfxSystemInstance->setMatrix(transform);

		mpPfxSystemInstance->tick(fdt, true);
	}
}

//*****************************************************************************
void VuUIPfxEntity::calcTransform(VuMatrix &transform)
{
	const VuCamera &camera = VuViewportManager::IF()->getUiCamera();

	VuUIDrawParams uiDrawParams;
	VuUIDrawUtil::getParams(this, uiDrawParams);

	VuVector2 screenPos = uiDrawParams.transform(mPfxPos);
	mAnchor.apply(screenPos, screenPos);
	screenPos = VuUI::IF()->getCropMatrix().transform(screenPos);
	float dist = (mPfxDist - camera.getNearPlane())/(camera.getFarPlane() - camera.getNearPlane());
	VuVector3 worldPos = camera.screenToWorld(VuVector3(screenPos.mX, screenPos.mY, dist));

	transform = camera.getTransform();
	transform.setTrans(worldPos);
}
