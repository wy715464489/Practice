//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  BB10 touchscreen hardware abstration layer
//
//*****************************************************************************

#include <screen/screen.h>
#include "VuBB10Touch.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTouch, VuBB10Touch);


//*****************************************************************************
VuBB10Touch::VuBB10Touch()
{
}

//*****************************************************************************
void VuBB10Touch::onTouchEvent(int type, VUUINT32 id, int x, int y)
{
	int displayWidth, displayHeight;
	VuGfx::IF()->getDisplaySize(displayWidth, displayHeight);
	VuVector2 pos((float)x/displayWidth, (float)y/displayHeight);

	if ( type == SCREEN_EVENT_MTOUCH_TOUCH )
	{
		for ( int i = 0; i < mTouchArray.size(); i++ )
		{
			if ( mTouchArray[i].mId == id )
			{
				VUASSERT(0, "Touch mismatch");
				return;
			}
		}
	
		VuTouchEntry entry;
		entry.mId = id;
		entry.mPos = pos;
		mTouchArray.push_back(entry);
	
		onTouchDownInternal(entry.mPos);
	}
	else if ( type == SCREEN_EVENT_MTOUCH_MOVE )
	{
		for ( int i = 0; i < mTouchArray.size(); i++ )
		{
			if ( mTouchArray[i].mId == id )
			{
				mTouchArray[i].mPos = pos;
				onTouchMoveInternal();
				return;
			}
		}
	}
	else if ( type == SCREEN_EVENT_MTOUCH_RELEASE )
	{
		for ( int i = 0; i < mTouchArray.size(); i++ )
		{
			if ( mTouchArray[i].mId == id )
			{
				onTouchUpInternal(mTouchArray[i].mPos);
				mTouchArray.eraseSwap(i);
				return;
			}
		}
	}
}
