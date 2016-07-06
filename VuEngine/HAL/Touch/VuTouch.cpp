//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Touchscreen hardware abstration layer
//
//*****************************************************************************

#include "VuTouch.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
VuTouch::VuTouch():
	mFocusPriority(0)
{
}

//*****************************************************************************
void VuTouch::addCallback(Callback *pCB)
{
	mCallbacks.push_back(VuCallbackEntry(pCB));
}

//*****************************************************************************
void VuTouch::removeCallback(Callback *pCB)
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
	{
		if ( iter->mpCallback == pCB )
		{
			mCallbacks.erase(iter);
			break;
		}
	}

	recalculateFocusPriority();
}

//*****************************************************************************
void VuTouch::setCallbackPriority(Callback *pCB, VUUINT32 priority)
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mpCallback == pCB )
			iter->mPriority = priority;

	recalculateFocusPriority();
}

//*****************************************************************************
void VuTouch::addLowLevelCallback(Callback *pCB)
{
	mLowLevelCallbacks.push_back(VuCallbackEntry(pCB));
}

//*****************************************************************************
void VuTouch::removeLowLevelCallback(Callback *pCB)
{
	for ( Callbacks::iterator iter = mLowLevelCallbacks.begin(); iter != mLowLevelCallbacks.end(); iter++ )
	{
		if ( iter->mpCallback == pCB )
		{
			mLowLevelCallbacks.erase(iter);
			break;
		}
	}
}

//*****************************************************************************
bool VuTouch::hasFocus(Callback *pCB)
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mpCallback == pCB )
			return iter->mPriority == mFocusPriority;

	return false;
}

//*****************************************************************************
void VuTouch::getTouch(int index, VuVector2 &touch)
{
	getTouchRaw(index, touch);
	touch = VuMathUtil::unapplySafeZone(touch, VuViewportManager::IF()->getSafeZone());
}

//*****************************************************************************
void VuTouch::onTouchDownInternal(const VuVector2 &touch)
{
	VuVector2 safeTouch = VuMathUtil::unapplySafeZone(touch, VuViewportManager::IF()->getSafeZone());

	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mPriority == mFocusPriority )
			iter->mpCallback->onTouchDown(safeTouch);

	for ( Callbacks::iterator iter = mLowLevelCallbacks.begin(); iter != mLowLevelCallbacks.end(); iter++ )
		iter->mpCallback->onTouchDown(safeTouch);
}

//*****************************************************************************
void VuTouch::onTouchUpInternal(const VuVector2 &touch)
{
	VuVector2 safeTouch = VuMathUtil::unapplySafeZone(touch, VuViewportManager::IF()->getSafeZone());

	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mPriority == mFocusPriority )
			iter->mpCallback->onTouchUp(safeTouch);

	for ( Callbacks::iterator iter = mLowLevelCallbacks.begin(); iter != mLowLevelCallbacks.end(); iter++ )
		iter->mpCallback->onTouchUp(safeTouch);
}

//*****************************************************************************
void VuTouch::onTouchMoveInternal()
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mPriority == mFocusPriority )
			iter->mpCallback->onTouchMove();

	for ( Callbacks::iterator iter = mLowLevelCallbacks.begin(); iter != mLowLevelCallbacks.end(); iter++ )
		iter->mpCallback->onTouchMove();
}

//*****************************************************************************
void VuTouch::onTouchSpecialInternal(eSpecial special)
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mPriority == mFocusPriority )
			iter->mpCallback->onTouchSpecial(special);

	for ( Callbacks::iterator iter = mLowLevelCallbacks.begin(); iter != mLowLevelCallbacks.end(); iter++ )
		iter->mpCallback->onTouchSpecial(special);
}

//*****************************************************************************
void VuTouch::recalculateFocusPriority()
{
	mFocusPriority = 0;
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		mFocusPriority = VuMax(mFocusPriority, iter->mPriority);
}