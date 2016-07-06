//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 touchscreen hardware abstration layer
//
//*****************************************************************************

#include "VuWin32Touch.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTouch, VuWin32Touch);


//*****************************************************************************
VuWin32Touch::VuWin32Touch():
	mMouseDown(0),
	mMousePos(0,0)
{
}

//*****************************************************************************
void VuWin32Touch::onMouseEvent(int type, float x1, float y1)
{
	mMousePos = VuVector2(x1, y1);

	if ( type == WM_LBUTTONDOWN )
	{
		mMouseDown = 1;
		onTouchDownInternal(mMousePos);
	}
	else if ( type == WM_LBUTTONUP )
	{
		mMouseDown = 0;
		onTouchUpInternal(mMousePos);
	}
	else if ( type == WM_RBUTTONDOWN )
	{
#ifndef VURETAIL
		onTouchSpecialInternal(SPECIAL_BACK_PRESSED);
#endif
	}
	else if ( type == WM_MOUSEMOVE )
	{
		onTouchMoveInternal();
	}
}