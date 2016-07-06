//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuUITouchSwipeEntity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/UI/VuUIPropertyUtil.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/UI/VuUIInputUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"


class VuUITouchSwipeEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUITouchSwipeEntity();

protected:
	// scripting
	VuRetVal			Enable(const VuParams &params)	{ mEnabled = true; return VuRetVal(); }
	VuRetVal			Disable(const VuParams &params)	{ mEnabled = false; return VuRetVal(); }

	// event handlers
	void				OnUITick(const VuParams &params);
	void				OnUITouch(const VuParams &params);

	void				drawLayout(bool bSelected);
	bool				executePlug(const char *plugName);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool					mEnabled;
	VuUIRectProperties		mTouchRect;
	VuUIAnchorProperties	mAnchor;
	float					mTouchDelta;

	bool				mNewTouch;
	bool				mTouchDown;
	bool				mTouchUp;
	VuVector2			mTouchDownPos;
	VuVector2			mTouchPos;

	enum eState { IDLE, DETECTING };
	eState				mState;
};

IMPLEMENT_RTTI(VuUITouchSwipeEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUITouchSwipeEntity);


//*****************************************************************************
VuUITouchSwipeEntity::VuUITouchSwipeEntity():
	mEnabled(true),
	mTouchDelta(50),
	mNewTouch(false),
	mTouchDown(false),
	mTouchUp(false),
	mState(IDLE)
{
	// properties
	addProperty(new VuBoolProperty("Enabled", mEnabled));
	addProperty(new VuRectProperty("Touch Rect", mTouchRect));
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");
	addProperty(new VuFloatProperty("Touch Delta", mTouchDelta));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 120));
	addComponent(new Vu2dLayoutComponent(this, &VuUITouchSwipeEntity::drawLayout));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUITouchSwipeEntity, Enable);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUITouchSwipeEntity, Disable);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Up);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Down);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Left);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Right);

	// event handlers
	REG_EVENT_HANDLER(VuUITouchSwipeEntity, OnUITick);
	REG_EVENT_HANDLER(VuUITouchSwipeEntity, OnUITouch);
}

//*****************************************************************************
void VuUITouchSwipeEntity::OnUITick(const VuParams &params)
{
	if ( mEnabled )
	{
		if ( mState == IDLE )
		{
			if ( mNewTouch )
			{
				VuUIDrawParams uiDrawParams;
				VuUIDrawUtil::getParams(this, uiDrawParams);

				VuRect touchRect = uiDrawParams.transform(mTouchRect);
				mAnchor.apply(touchRect, touchRect);

				if ( touchRect.contains(mTouchPos) )
				{
					mState = DETECTING;
				}
			}
		}
		else if ( mState == DETECTING )
		{
			if ( mTouchDown )
			{
				VuUIDrawParams uiDrawParams;
				VuUIDrawUtil::getParams(this, uiDrawParams);

				VuVector2 touchDelta = uiDrawParams.transformInv(mTouchPos) - uiDrawParams.transformInv(mTouchDownPos);

				if ( touchDelta.mX < -mTouchDelta )
				{
					if ( executePlug("Left") )
						mState = IDLE;
				}
				if ( (touchDelta.mX > mTouchDelta) && (mState == DETECTING) )
				{
					if ( executePlug("Right") )
						mState = IDLE;
				}
				else if ( (touchDelta.mY < -mTouchDelta) && (mState == DETECTING) )
				{
					if ( executePlug("Up") )
						mState = IDLE;
				}
				else if ( (touchDelta.mY > mTouchDelta) && (mState == DETECTING) )
				{
					if ( executePlug("Down") )
						mState = IDLE;
				}
			}
			else
			{
				mState = IDLE;
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
void VuUITouchSwipeEntity::OnUITouch(const VuParams &params)
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
				mNewTouch = true;
				mTouchDown = true;
				mTouchDownPos = touch;
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
void VuUITouchSwipeEntity::drawLayout(bool bSelected)
{
	if ( bSelected )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);
		VuRect touchRect = uiDrawParams.transform(mTouchRect);
		VuGfxUtil::IF()->drawRectangleOutline2d(0.0f, VuColor(255,255,255), touchRect);
	}
}

//*****************************************************************************
bool VuUITouchSwipeEntity::executePlug(const char *plugName)
{
	if ( VuScriptPlug *pPlug = mpScriptComponent->getPlug(plugName) )
	{
		if ( pPlug->getNumConnections() )
		{
			pPlug->execute();
			return true;
		}
	}

	return false;
}
