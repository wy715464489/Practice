//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Button class
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/UI/VuUIInputUtil.h"
#include "VuEngine/UI/VuUIPropertyUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"

struct VuUIDrawParams;


class VuUITouchButtonEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUITouchButtonEntity();

protected:
	// scripting
	VuRetVal			Enable(const VuParams &params)	{ mEnabled = true; return VuRetVal(); }
	VuRetVal			Disable(const VuParams &params)	{ mEnabled = false; return VuRetVal(); }

	// event handlers
	void				OnUITick(const VuParams &params);
	void				OnUITouch(const VuParams &params);

	void				drawLayout(bool bSelected);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool					mEnabled;
	VuUIRectProperties		mTouchRect;
	VuUIAnchorProperties	mAnchor;

	bool				mNewTouch;
	bool				mTouchDown;
	bool				mTouchUp;
	VuVector2			mTouchPos;
};

IMPLEMENT_RTTI(VuUITouchButtonEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUITouchButtonEntity);



//*****************************************************************************
VuUITouchButtonEntity::VuUITouchButtonEntity():
	mEnabled(true),
	mTouchRect(0,0,20,10),
	mNewTouch(false),
	mTouchDown(false),
	mTouchUp(false)
{
	// properties
	addProperty(new VuBoolProperty("Enabled", mEnabled));
	ADD_UI_RECT_PROPERTIES(getProperties(), mTouchRect, "Touch");
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 120));
	addComponent(new Vu2dLayoutComponent(this, &VuUITouchButtonEntity::drawLayout));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUITouchButtonEntity, Enable);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUITouchButtonEntity, Disable);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Pressed);

	// event handlers
	REG_EVENT_HANDLER(VuUITouchButtonEntity, OnUITick);
	REG_EVENT_HANDLER(VuUITouchButtonEntity, OnUITouch);
}

//*****************************************************************************
void VuUITouchButtonEntity::OnUITick(const VuParams &params)
{
	if ( mEnabled )
	{
		if ( mNewTouch )
		{
			VuUIDrawParams uiDrawParams;
			VuUIDrawUtil::getParams(this, uiDrawParams);

			VuRect touchRect = uiDrawParams.transform(mTouchRect);
			mAnchor.apply(touchRect, touchRect);

			if ( touchRect.contains(mTouchPos) )
			{
				mpScriptComponent->getPlug("Pressed")->execute();
			}
		}

		if ( mTouchUp )
		{
			mTouchDown = false;
			mTouchUp = false;
		}
		mNewTouch = false;
	}
}

//*****************************************************************************
void VuUITouchButtonEntity::OnUITouch(const VuParams &params)
{
	if ( mEnabled )
	{
		VuParams::VuAccessor accessor(params);
		int action = accessor.getInt();
		VuVector2 touch = accessor.getVector2();

		if ( action == VuUIInputUtil::TOUCH_DOWN )
		{
			if ( !mTouchDown )
			{
				mTouchDown = true;
				mNewTouch = true;
				mTouchPos = touch;
			}
		}
		else if ( action == VuUIInputUtil::TOUCH_UP )
		{
			mTouchUp = true;
		}
		else if ( action == VuUIInputUtil::TOUCH_MOVE )
		{
			mTouchPos = touch;
		}
	}
}

//*****************************************************************************
void VuUITouchButtonEntity::drawLayout(bool bSelected)
{
	if ( bSelected )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);
		VuRect touchRect = uiDrawParams.transform(mTouchRect);
		VuGfxUtil::IF()->drawRectangleOutline2d(0.0f, VuColor(255,255,255), touchRect);
	}
}
