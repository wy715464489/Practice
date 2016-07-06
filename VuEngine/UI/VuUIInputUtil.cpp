//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI input utility functionality.
// 
//*****************************************************************************

#include "VuUIInputUtil.h"
#include "VuUI.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Managers/VuInputManager.h"
#include "VuEngine/Managers/VuTickManager.h"


struct ChannelInfo
{
	bool		mRepeats;
	const char	*mButtonName;
};
static ChannelInfo sChannelInfo[] =
{
	{ false, "Select" }, // CHANNEL_SELECT,
	{ false, "Back" },   // CHANNEL_BACK,
	{ true,  "Up" },     // CHANNEL_UP,
	{ true,  "Down" },   // CHANNEL_DOWN,
	{ true,  "Left" },   // CHANNEL_LEFT,
	{ true,  "Right" },  // CHANNEL_RIGHT,
};
VU_COMPILE_TIME_ASSERT(sizeof(sChannelInfo)/sizeof(sChannelInfo[0]) == VuUIInputUtil::NUM_CHANNELS);


//*****************************************************************************
VuUIInputUtil::VuUIInputUtil(VuEntity *pOwner):
	mpOwner(pOwner),
	mEnabled(false),
	mPriority(0),
	mRepeatDelay(0.3f),
	mRepeatTimer(0.15f)
{
	for ( int iGamePad = 0; iGamePad < VuGamePad::MAX_NUM_PADS; iGamePad++ )
		for ( int iChannel = 0; iChannel < NUM_CHANNELS; iChannel++ )
			mRepeatTimers[iGamePad][iChannel] = mRepeatDelay;
}

//*****************************************************************************
VuUIInputUtil::~VuUIInputUtil()
{
	disable();
}

//*****************************************************************************
void VuUIInputUtil::enable()
{
	if ( !mEnabled )
	{
		mEnabled = true;

		VuTouch::IF()->addCallback(this);
		VuTouch::IF()->setCallbackPriority(this, mPriority);

		for ( int iGamePad = 0; iGamePad < VuGamePad::MAX_NUM_PADS; iGamePad++ )
			for ( int iChannel = 0; iChannel < NUM_CHANNELS; iChannel++ )
				mRepeatTimers[iGamePad][iChannel] = mRepeatDelay;
	}
}

//*****************************************************************************
void VuUIInputUtil::disable()
{
	if ( mEnabled )
	{
		mEnabled = false;

		VuTouch::IF()->removeCallback(this);
	}
}

//*****************************************************************************
void VuUIInputUtil::setPriority(VUUINT32 priority)
{
	mPriority = priority;
	if ( mEnabled )
		VuTouch::IF()->setCallbackPriority(this, priority);
}

//*****************************************************************************
void VuUIInputUtil::tick(VUUINT32 padMask)
{
	if ( mEnabled && VuTouch::IF()->hasFocus(this) )
	{
		float fdt = VuTickManager::IF()->getRealDeltaTime();

		if ( padMask )
		{
			for ( int iChannel = 0; iChannel < NUM_CHANNELS; iChannel++ )
			{
				for ( int padIndex = 0; padIndex < VuGamePad::MAX_NUM_PADS; padIndex++ )
				{
					if ( (1<<padIndex) & padMask )
					{
						const char *buttonName = sChannelInfo[iChannel].mButtonName;
						bool buttonWasPressed = VuInputManager::IF()->getButtonWasPressed(padIndex, buttonName);
						bool buttonWasReleased = VuInputManager::IF()->getButtonWasReleased(padIndex, buttonName);
						bool buttonDown = VuInputManager::IF()->getButtonValue(padIndex, buttonName);

						// repeat
						if ( sChannelInfo[iChannel].mRepeats )
						{
							if ( buttonDown )
							{
								mRepeatTimers[padIndex][iChannel] -= fdt;
								if ( mRepeatTimers[padIndex][iChannel] < 0.0f )
								{
									buttonWasPressed = true;
									buttonWasReleased = true;
									mRepeatTimers[padIndex][iChannel] = mRepeatTimer;
								}
							}
							else
							{
								mRepeatTimers[padIndex][iChannel] = mRepeatDelay;
							}
						}

						if ( buttonWasPressed )
							sendGamePadEvent(iChannel, true, padIndex);

						if ( buttonWasReleased )
							sendGamePadEvent(iChannel, false, padIndex);
					}
				}
			}
		}
	}
}

//*****************************************************************************
void VuUIInputUtil::sendGamePadEvent(int channel, bool down, int padIndex)
{
	VuParams params;
	params.addInt(channel);
	params.addBool(down);
	params.addInt(padIndex);

	mpOwner->handleEventRecursive("OnUIGamePad", params);
}

//*****************************************************************************
void VuUIInputUtil::onTouchDown(const VuVector2 &touch)
{
	VuVector2 transformedTouch = VuUI::IF()->getInvCropMatrix().transform(touch);

	VuParams params;
	params.addInt(TOUCH_DOWN);
	params.addVector2(transformedTouch);

	mpOwner->handleEventRecursive("OnUITouch", params);
}

//*****************************************************************************
void VuUIInputUtil::onTouchUp(const VuVector2 &touch)
{
	VuVector2 transformedTouch = VuUI::IF()->getInvCropMatrix().transform(touch);

	VuParams params;
	params.addInt(TOUCH_UP);
	params.addVector2(transformedTouch);

	mpOwner->handleEventRecursive("OnUITouch", params);
}

//*****************************************************************************
void VuUIInputUtil::onTouchMove()
{
	VuVector2 touch;
	VuTouch::IF()->getTouch(0, touch);

	VuVector2 transformedTouch = VuUI::IF()->getInvCropMatrix().transform(touch);

	VuParams params;
	params.addInt(TOUCH_MOVE);
	params.addVector2(transformedTouch);

	mpOwner->handleEventRecursive("OnUITouch", params);
}

//*****************************************************************************
void VuUIInputUtil::onTouchSpecial(VuTouch::eSpecial special)
{
	VuParams params;
	params.addInt(special);

	mpOwner->handleEventRecursive("OnUITouchSpecial", params);
}
