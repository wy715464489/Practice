//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android touchscreen hardware abstration layer
//
//*****************************************************************************

#include "VuAndroidTouch.h"


/* Bit shift for the action bits holding the pointer index as
 * defined by AMOTION_EVENT_ACTION_POINTER_INDEX_MASK.
 */
#define AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT 8

enum {
    /* Bit mask of the parts of the action code that are the action itself.
     */
    AMOTION_EVENT_ACTION_MASK = 0xff,

    /* Bits in the action code that represent a pointer index, used with
     * AMOTION_EVENT_ACTION_POINTER_DOWN and AMOTION_EVENT_ACTION_POINTER_UP.  Shifting
     * down by AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT provides the actual pointer
     * index where the data for the pointer going up or down can be found.
     */
    AMOTION_EVENT_ACTION_POINTER_INDEX_MASK  = 0xff00,

    /* A pressed gesture has started, the motion contains the initial starting location.
     */
    AMOTION_EVENT_ACTION_DOWN = 0,

    /* A pressed gesture has finished, the motion contains the final release location
     * as well as any intermediate points since the last down or move event.
     */
    AMOTION_EVENT_ACTION_UP = 1,

    /* A change has happened during a press gesture (between AMOTION_EVENT_ACTION_DOWN and
     * AMOTION_EVENT_ACTION_UP).  The motion contains the most recent point, as well as
     * any intermediate points since the last down or move event.
     */
    AMOTION_EVENT_ACTION_MOVE = 2,

    /* The current gesture has been aborted.
     * You will not receive any more points in it.  You should treat this as
     * an up event, but not perform any action that you normally would.
     */
    AMOTION_EVENT_ACTION_CANCEL = 3,

    /* A movement has happened outside of the normal bounds of the UI element.
     * This does not provide a full gesture, but only the initial location of the movement/touch.
     */
    AMOTION_EVENT_ACTION_OUTSIDE = 4,

    /* A non-primary pointer has gone down.
     * The bits in AMOTION_EVENT_ACTION_POINTER_INDEX_MASK indicate which pointer changed.
     */
    AMOTION_EVENT_ACTION_POINTER_DOWN = 5,

    /* A non-primary pointer has gone up.
     * The bits in AMOTION_EVENT_ACTION_POINTER_INDEX_MASK indicate which pointer changed.
     */
    AMOTION_EVENT_ACTION_POINTER_UP = 6
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTouch, VuAndroidTouch);


//*****************************************************************************
VuAndroidTouch::VuAndroidTouch():
	mTouchCount(0)
{
	memset(mPointers, 0, sizeof(mPointers));
	memset(mTouchArray, 0, sizeof(mTouchArray));
}

//*****************************************************************************
void VuAndroidTouch::onTouchEvent(int action, int pointerBits, float x1, float y1, float x2, float y2)
{
	// update coords
	if ( pointerBits & 0x1 ) mPointers[0].mPos = VuVector2(x1, y1);
	if ( pointerBits & 0x2 ) mPointers[1].mPos = VuVector2(x2, y2);

	int maskedAction = (action & AMOTION_EVENT_ACTION_MASK);
	int pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

	switch ( maskedAction )
	{
		case AMOTION_EVENT_ACTION_DOWN:
		{
			if ( pointerBits & 0x1 )
			{
				mPointers[0].mDown = true;
				onTouchDownInternal(mPointers[0].mPos);
			}
			if ( pointerBits & 0x2 )
			{
				mPointers[1].mDown = true;
				onTouchDownInternal(mPointers[1].mPos);
			}
			break;
		}
		case AMOTION_EVENT_ACTION_UP:
		{
			if ( mPointers[0].mDown )
			{
				mPointers[0].mDown = false;
				onTouchUpInternal(mPointers[0].mPos);
			}
			if ( mPointers[1].mDown )
			{
				mPointers[1].mDown = false;
				onTouchUpInternal(mPointers[1].mPos);
			}
			break;
		}
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
		{
			if ( pointerIndex < 2 )
			{
				mPointers[pointerIndex].mDown = true;
				onTouchDownInternal(mPointers[pointerIndex].mPos);
			}
			break;
		}
		case AMOTION_EVENT_ACTION_POINTER_UP:
		{
			if ( pointerIndex < 2 )
			{
				mPointers[pointerIndex].mDown = false;
				onTouchUpInternal(mPointers[pointerIndex].mPos);
			}
			break;
		}
		case AMOTION_EVENT_ACTION_MOVE:
		{
			onTouchMoveInternal();
			break;
		}
	}

	mTouchCount = 0;
	for ( int i = 0; i < 2; i++ )
		if ( mPointers[i].mDown )
			mTouchArray[mTouchCount++] = mPointers[i].mPos;

	//__android_log_print(ANDROID_LOG_DEBUG, "TOUCH", "%d %d %x %f %f %f %f (%d)\n", maskedAction, pointerIndex, pointerBits, x1, y1, x2, y2, mTouchCount);
}

//*****************************************************************************
void VuAndroidTouch::onTouchSpecial(eSpecial special)
{
	onTouchSpecialInternal(special);
}