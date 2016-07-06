//*****************************************************************************
//
//  Copyright (c) 2011-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows touchscreen hardware abstration layer
//
//*****************************************************************************

#include "VuWindowsTouch.h"
#include "VuEngine/Events/VuEventManager.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTouch, VuWindowsTouch);


//*****************************************************************************
VuWindowsTouch::VuWindowsTouch()
{
	// register event handlers
	REG_EVENT_HANDLER(VuWindowsTouch, OnWindowsPointerDown);
	REG_EVENT_HANDLER(VuWindowsTouch, OnWindowsPointerUp);
	REG_EVENT_HANDLER(VuWindowsTouch, OnWindowsPointerMove);
	REG_EVENT_HANDLER(VuWindowsTouch, OnWindowsTouchSpecial);
}

//*****************************************************************************
void VuWindowsTouch::onPointerDown(VUUINT32 id, float x1, float y1)
{
	VuParams params;
	params.addUnsignedInt(id);
	params.addVector2(VuVector2(x1, y1));
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnWindowsPointerDown", params);
}

//*****************************************************************************
void VuWindowsTouch::onPointerUp(VUUINT32 id, float x1, float y1)
{
	VuParams params;
	params.addUnsignedInt(id);
	params.addVector2(VuVector2(x1, y1));
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnWindowsPointerUp", params);
}

//*****************************************************************************
void VuWindowsTouch::onPointerMove(VUUINT32 id, float x1, float y1)
{
	VuParams params;
	params.addUnsignedInt(id);
	params.addVector2(VuVector2(x1, y1));
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnWindowsPointerMove", params);
}

//*****************************************************************************
void VuWindowsTouch::onTouchSpecial(eSpecial special)
{
	VuParams params;
	params.addUnsignedInt(special);
	VuEventManager::IF()->broadcastDelayed(0.0f, true, "OnWindowsTouchSpecial", params);
}

//*****************************************************************************
void VuWindowsTouch::OnWindowsPointerDown(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	VUUINT32 id = accessor.getUnsignedInt();
	VuVector2 pos = accessor.getVector2();

	// add touch
	TouchPoint *pTouchPoint = findTouchPoint(id);
	if ( pTouchPoint == VUNULL )
	{
		mTouchPoints.resize(mTouchPoints.size() + 1);
		pTouchPoint = &mTouchPoints.back();
	}
	
	pTouchPoint->mID = id;
	pTouchPoint->mPos = pos;

	onTouchDownInternal(pos);
}

//*****************************************************************************
void VuWindowsTouch::OnWindowsPointerUp(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	VUUINT32 id = accessor.getUnsignedInt();
	VuVector2 pos = accessor.getVector2();

	// erase touch
	if ( TouchPoint *pTouchPoint = findTouchPoint(id) )
		mTouchPoints.eraseSwap(int(pTouchPoint - &mTouchPoints.begin()));

	onTouchUpInternal(pos);
}

//*****************************************************************************
void VuWindowsTouch::OnWindowsPointerMove(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	VUUINT32 id = accessor.getUnsignedInt();
	VuVector2 pos = accessor.getVector2();

	// update touch
	if ( TouchPoint *pTouchPoint = findTouchPoint(id) )
	{
		pTouchPoint->mPos = pos;
		onTouchMoveInternal();
	}
}

//*****************************************************************************
void VuWindowsTouch::OnWindowsTouchSpecial(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	eSpecial special = eSpecial(accessor.getUnsignedInt());

	onTouchSpecialInternal(special);
}

//*****************************************************************************
VuWindowsTouch::TouchPoint *VuWindowsTouch::findTouchPoint(VUUINT32 id)
{
	for ( TouchPoint *pTouchPoint = &mTouchPoints.begin(); pTouchPoint < &mTouchPoints.end(); pTouchPoint++ )
		if ( pTouchPoint->mID == id )
			return pTouchPoint;

	return VUNULL;
}