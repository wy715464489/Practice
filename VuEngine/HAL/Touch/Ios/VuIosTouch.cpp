//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios touchscreen hardware abstration layer
//
//*****************************************************************************

#include "VuIosTouch.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTouch, VuIosTouch);


//*****************************************************************************
VuIosTouch::VuIosTouch()
{
}

//*****************************************************************************
void VuIosTouch::onTouchDown(void *pTouch, float x, float y)
{
	for ( int i = 0; i < mTouchArray.size(); i++ )
	{
		if ( mTouchArray[i].mpTouch == pTouch )
		{
			VUASSERT(0, "Touch mismatch");
			return;
		}
	}
	
	VuTouchEntry entry;
	entry.mpTouch = pTouch;
	entry.mPos = VuVector2(x, y);
	mTouchArray.push_back(entry);
	
	onTouchDownInternal(entry.mPos);
}

//*****************************************************************************
void VuIosTouch::onTouchMove(void *pTouch, float x, float y)
{
	for ( int i = 0; i < mTouchArray.size(); i++ )
	{
		if ( mTouchArray[i].mpTouch == pTouch )
		{
			mTouchArray[i].mPos = VuVector2(x, y);
			onTouchMoveInternal();
			return;
		}
	}
}

//*****************************************************************************
void VuIosTouch::onTouchUp(void *pTouch, float x, float y)
{
	for ( int i = 0; i < mTouchArray.size(); i++ )
	{
		if ( mTouchArray[i].mpTouch == pTouch )
		{
			onTouchUpInternal(mTouchArray[i].mPos);
			mTouchArray.eraseSwap(i);
			return;
		}
	}
}

//*****************************************************************************
void VuIosTouch::onTouchCancel(void *pTouch, float x, float y)
{
	for ( int i = 0; i < mTouchArray.size(); i++ )
	{
		if ( mTouchArray[i].mpTouch == pTouch )
		{
			mTouchArray.eraseSwap(i);
			return;
		}
	}
}
