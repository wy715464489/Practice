//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout class
// 
//*****************************************************************************

#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/UI/VuUIInputUtil.h"
#include "VuEngine/UI/VuUIPropertyUtil.h"
#include "VuEngine/UI/PageLayout/VuUIPageLayout.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Math/VuMathUtil.h"

class VuScriptComponent;


class VuUIPageLayoutEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUIPageLayoutEntity();

	virtual void		onPostLoad() { layoutModified(); }
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// event handlers
	void				OnUITick(const VuParams &params);
	void				OnUITouch(const VuParams &params);
	void				OnUIDraw(const VuParams &params);

	VuRetVal			Show(const VuParams &params)		{ mbVisible = true; return VuRetVal(); }
	VuRetVal			Hide(const VuParams &params)		{ mbVisible = false; return VuRetVal(); }
	VuRetVal			StartScroll(const VuParams &params);
	VuRetVal			StopScroll(const VuParams &params);
	VuRetVal			ResetScroll(const VuParams &params);

	void				drawLayout(bool bSelected);
	void				drawPage(float alpha);

	void				layoutModified();

	float				calcScrollMax();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool					mbVisible;
	VuUIRectProperties		mRect;
	VuUIAnchorProperties	mAnchor;
	std::string				mPageLayoutEntry;
	bool					mbScrollAtStart;
	float					mScrollSpeed;
	float					mScrollLag;
	float					mScrollResidualDamping;
	bool					mTouchable;
	bool					mScrolling;

	VuDBEntryProperty	*mpPageLayoutProperty;
	VuUIPageLayout		mPageLayout;
	float				mPageLayoutHeight;
	bool				mbScrolling;
	bool				mbReachedEnd;

	bool				mTouchScrollActive;
	VuVector2			mInitialTouchDown;
	float				mScrollTouchPos;
	float				mScrollPos;
	float				mScrollTargetPos;
	float				mScrollVel;
	float				mScrollResidualVel;
};


IMPLEMENT_RTTI(VuUIPageLayoutEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIPageLayoutEntity);


//*****************************************************************************
VuUIPageLayoutEntity::VuUIPageLayoutEntity():
	mbVisible(true),
	mRect(0,0,100,100),
	mbScrollAtStart(false),
	mScrollSpeed(20.0f),
	mScrollLag(0.1f),
	mScrollResidualDamping(0.99f),
	mTouchable(true),
	mScrolling(true),
	mPageLayoutHeight(0.0f),
	mbScrolling(false),
	mbReachedEnd(false),
	mTouchScrollActive(false),
	mScrollPos(0),
	mScrollTargetPos(0),
	mScrollVel(0),
	mScrollResidualVel(0)
{
	// properties
	addProperty(new VuBoolProperty("Visible", mbVisible));
	ADD_UI_RECT_PROPERTIES(getProperties(), mRect, "");
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");
	addProperty(mpPageLayoutProperty = new VuDBEntryProperty("Page Layout", mPageLayoutEntry, "PageLayoutDB"));
	addProperty(new VuBoolProperty("Scroll at Start", mbScrollAtStart));
	addProperty(new VuFloatProperty("Scroll Speed", mScrollSpeed));
	addProperty(new VuFloatProperty("Scroll Lag", mScrollLag));
	addProperty(new VuFloatProperty("Scroll Residual Damping", mScrollResidualDamping));
	addProperty(new VuBoolProperty("Touchable", mTouchable));
	addProperty(new VuBoolProperty("Scrolling", mScrolling));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));
	addComponent(new Vu2dLayoutComponent(this, &VuUIPageLayoutEntity::drawLayout));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPageLayoutEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPageLayoutEntity, Hide);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPageLayoutEntity, StartScroll);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPageLayoutEntity, StopScroll);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIPageLayoutEntity, ResetScroll);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnReachedEnd);

	// event handlers
	REG_EVENT_HANDLER(VuUIPageLayoutEntity, OnUITick);
	REG_EVENT_HANDLER(VuUIPageLayoutEntity, OnUITouch);
	REG_EVENT_HANDLER(VuUIPageLayoutEntity, OnUIDraw);

	mpPageLayoutProperty->setWatcher(this, &VuUIPageLayoutEntity::layoutModified);
}

//*****************************************************************************
void VuUIPageLayoutEntity::onGameInitialize()
{
	mScrollPos = 0.0f;
	mScrollTargetPos = 0.0f;
	mScrollVel = 0.0f;
	mScrollResidualVel = 0.0f;

	if ( mbScrollAtStart )
	{
		mbScrolling = true;
	}
}

//*****************************************************************************
void VuUIPageLayoutEntity::onGameRelease()
{
}

//*****************************************************************************
void VuUIPageLayoutEntity::OnUITick(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	float fdt = accessor.getFloat();

	// use real dt
	fdt = VuTickManager::IF()->getRealDeltaTime();

	if ( mbScrolling && !mbReachedEnd )
	{
		if ( !mTouchScrollActive )
		{
			mScrollResidualVel *= VuPow(1.0f - mScrollResidualDamping, fdt);

			mScrollTargetPos += (mScrollSpeed + mScrollResidualVel)*fdt;
		}

		mScrollPos = VuMathUtil::smoothCD(mScrollPos, mScrollTargetPos, mScrollVel, mScrollLag, fdt);

		if ( mScrollPos > calcScrollMax() )
		{
			mbReachedEnd = true;
			mpScriptComponent->getPlug("OnReachedEnd")->execute();
		}
	}
}

//*****************************************************************************
void VuUIPageLayoutEntity::OnUITouch(const VuParams &params)
{
	if ( mTouchable )
	{
		VuParams::VuAccessor accessor(params);
		int action = accessor.getInt();
		VuVector2 touch = accessor.getVector2();

		if ( action == VuUIInputUtil::TOUCH_DOWN )
		{
			VuUIDrawParams uiDrawParams;
			VuUIDrawUtil::getParams(this, uiDrawParams);

			VuRect screenRect = (mRect + uiDrawParams.mPosition)*uiDrawParams.mInvAuthScale;
			mAnchor.apply(screenRect, screenRect);
			if ( screenRect.contains(touch) )
			{
				mTouchScrollActive = true;
				mInitialTouchDown = touch;
				mScrollTouchPos = mScrollTargetPos;
			}
		}
		else if ( action == VuUIInputUtil::TOUCH_UP )
		{
			mTouchScrollActive = false;
			mScrollResidualVel = mScrollVel;
		}
		else if ( action == VuUIInputUtil::TOUCH_MOVE )
		{
			if ( mTouchScrollActive )
			{
				VuUIDrawParams uiDrawParams;
				VuUIDrawUtil::getParams(this, uiDrawParams);

				VuVector2 touchDelta = (touch - mInitialTouchDown)/uiDrawParams.mInvAuthScale;

				mScrollTargetPos = mScrollTouchPos - touchDelta.mY;
				mScrollTargetPos = VuClamp(mScrollTargetPos, 0.0f, calcScrollMax());
			}
		}
	}
}

//*****************************************************************************
void VuUIPageLayoutEntity::OnUIDraw(const VuParams &params)
{
	float alpha = 1.0f;

	if ( mbVisible )
	{
		drawPage(alpha);
	}
}

//*****************************************************************************
VuRetVal VuUIPageLayoutEntity::StartScroll(const VuParams &params)
{
	mbScrolling = true;

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuUIPageLayoutEntity::StopScroll(const VuParams &params)
{
	mbScrolling = false;

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuUIPageLayoutEntity::ResetScroll(const VuParams &params)
{
	mScrollPos = 0.0f;
	mScrollTargetPos = 0.0f;
	mScrollVel = 0.0f;
	mScrollResidualVel = 0.0f;
	mbReachedEnd = false;

	return VuRetVal();
}

//*****************************************************************************
void VuUIPageLayoutEntity::drawLayout(bool bSelected)
{
	// draw rect
	if ( bSelected )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);

		VuRect screenRect = (mRect + uiDrawParams.mPosition)*uiDrawParams.mInvAuthScale;
		mAnchor.apply(screenRect, screenRect);
		VuGfxUtil::IF()->drawRectangleOutline2d(uiDrawParams.mDepth, VuColor(255,255,255), screenRect);
	}

	// draw text
	drawPage(1.0f);
}

//*****************************************************************************
void VuUIPageLayoutEntity::drawPage(float alpha)
{
	VuUIDrawParams uiDrawParams;
	VuUIDrawUtil::getParams(this, uiDrawParams);

	VuRect screenRect = (mRect + uiDrawParams.mPosition)*uiDrawParams.mInvAuthScale;
	mAnchor.apply(screenRect, screenRect);
	VuRect rect = screenRect/uiDrawParams.mInvAuthScale;

	float offsetY = 0.0f;
	if ( mScrolling )
		offsetY = VuLerp(rect.mHeight, -mPageLayoutHeight/uiDrawParams.mInvAuthScale.mY, mScrollPos/calcScrollMax());

	mPageLayout.draw(uiDrawParams.mDepth, screenRect, offsetY*uiDrawParams.mInvAuthScale.mY, alpha, uiDrawParams.mInvAuthScale);
}

//*****************************************************************************
void VuUIPageLayoutEntity::layoutModified()
{
	VuUIDrawParams uiDrawParams;
	VuUIDrawUtil::getParams(this, uiDrawParams);

	mPageLayout.setLayout(mpPageLayoutProperty->getEntryData());
	mPageLayoutHeight = mPageLayout.measureHeight(mRect.mWidth*uiDrawParams.mInvAuthScale.mX, uiDrawParams.mInvAuthScale);
}

//*****************************************************************************
float VuUIPageLayoutEntity::calcScrollMax()
{
	VuUIDrawParams uiDrawParams;
	VuUIDrawUtil::getParams(this, uiDrawParams);

	VuRect screenRect = mRect*uiDrawParams.mInvAuthScale;
	mAnchor.apply(screenRect, screenRect);
	VuRect rect = screenRect/uiDrawParams.mInvAuthScale;

	return rect.mHeight + mPageLayoutHeight/uiDrawParams.mInvAuthScale.mY;
}
