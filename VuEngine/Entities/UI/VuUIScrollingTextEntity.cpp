//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Scrolling Text class
// 
//*****************************************************************************

#include "VuUIScrollingTextEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"


IMPLEMENT_RTTI(VuUIScrollingTextEntity, VuUITextEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIScrollingTextEntity);


//*****************************************************************************
VuUIScrollingTextEntity::VuUIScrollingTextEntity():
	mbScrollAtStart(true),
	mScrollSpeed(20.0f),
	mbScrolling(false),
	mbReachedEnd(false)
{
	// properties
	addProperty(new VuBoolProperty("Scroll at Start", mbScrollAtStart));
	addProperty(new VuFloatProperty("Scroll Speed", mScrollSpeed));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIScrollingTextEntity, StartScroll);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIScrollingTextEntity, StopScroll);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUIScrollingTextEntity, ResetScroll);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnReachedEnd);

	// event handlers
	REG_EVENT_HANDLER(VuUIScrollingTextEntity, OnUITick);
}

//*****************************************************************************
void VuUIScrollingTextEntity::onGameInitialize()
{
	mbScrolling = mbScrollAtStart;
}

//*****************************************************************************
void VuUIScrollingTextEntity::OnUITick(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	float fdt = accessor.getFloat();

	// use real dt
	fdt = VuTickManager::IF()->getRealDeltaTime();

	if ( mbScrolling && !mbReachedEnd )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);

		mOffset.mY -= mScrollSpeed*fdt;

		VuVector2 size = measureString();
		if ( (-mOffset.mY) > size.mY*uiDrawParams.mAuthScale.mY )
		{
			mbReachedEnd = true;
			mpScriptComponent->getPlug("OnReachedEnd")->execute();
		}
	}
}

//*****************************************************************************
VuRetVal VuUIScrollingTextEntity::StartScroll(const VuParams &params)
{
	mbScrolling = true;

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuUIScrollingTextEntity::StopScroll(const VuParams &params)
{
	mbScrolling = false;

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuUIScrollingTextEntity::ResetScroll(const VuParams &params)
{
	mOffset.mY = 0.0f;
	mbReachedEnd = false;

	return VuRetVal();
}
