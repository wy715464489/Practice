//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Control class
// 
//*****************************************************************************

#include "VuAnimationControl.h"
#include "VuAnimation.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAnimationAsset.h"
#include "VuEngine/Assets/VuTimedEventAsset.h"


//*****************************************************************************
VuAnimationControl::VuAnimationControl(const std::string &assetName):
	mbLooping(true),
	mTimeFactor(1.0f),
	mLocalTime(0.0f),
	mWeight(1.0f),
	mpTimedEventAsset(VUNULL),
	mpEventIF(VUNULL)
{
	mpAnimationAsset = VuAssetFactory::IF()->createAsset<VuAnimationAsset>(assetName);
	mpAnimation = mpAnimationAsset->getAnimation();

	if ( mpAnimation )
		mpAnimation->addRef();
}

//*****************************************************************************
VuAnimationControl::VuAnimationControl(VuAnimation *pAnimation):
	mpAnimationAsset(VUNULL),
	mpAnimation(pAnimation),
	mbLooping(true),
	mTimeFactor(1.0f),
	mLocalTime(0.0f),
	mWeight(1.0f),
	mpTimedEventAsset(VUNULL),
	mpEventIF(VUNULL)
{
	mpAnimation->addRef();
}

//*****************************************************************************
VuAnimationControl::~VuAnimationControl()
{
	if ( mpAnimation )
		mpAnimation->removeRef();

	if ( mpAnimationAsset )
		VuAssetFactory::IF()->releaseAsset(mpAnimationAsset);

	if ( mpTimedEventAsset )
		VuAssetFactory::IF()->releaseAsset(mpTimedEventAsset);
}

//*****************************************************************************
void VuAnimationControl::setLooping(bool bLooping)
{
	mbLooping = bLooping;
	advance(0);
}

//*****************************************************************************
void VuAnimationControl::advance(float fdt)
{
	float prevTime = mLocalTime;

	mLocalTime += mTimeFactor*fdt;

	if ( mbLooping )
	{
		while ( mLocalTime >= mpAnimation->getTotalTime() )
			mLocalTime -= mpAnimation->getTotalTime();
		while ( mLocalTime < 0.0f )
			mLocalTime += mpAnimation->getTotalTime();
	}
	else
	{
		mLocalTime = VuClamp(mLocalTime, 0.0f, mpAnimation->getEndTime());
	}

	// handle animation events
	if ( prevTime != mLocalTime )
	{
		if ( mpEventIF )
		{
			bool bHandleTimedEvents = mpTimedEventAsset && mpTimedEventAsset->getEventCount();
			if ( fdt > 0 )
			{
				if ( mLocalTime > prevTime )
				{
					if ( bHandleTimedEvents )
					{
						handleTimedEventsForward(prevTime, mLocalTime);
					}
					if ( mLocalTime == mpAnimation->getEndTime() )
						mpEventIF->onAnimationEvent("AnimDone", VuJsonContainer::null);
				}
				else
				{
					if ( bHandleTimedEvents )
					{
						handleTimedEventsForward(prevTime, mpAnimation->getTotalTime());
						handleTimedEventsForward(0.0f, mLocalTime);
					}
					mpEventIF->onAnimationEvent("AnimLoop", VuJsonContainer::null);
				}
			}
			else if ( fdt < 0 )
			{
				if ( mLocalTime < prevTime )
				{
					if ( bHandleTimedEvents )
					{
						handleTimedEventsReverse(prevTime, mLocalTime);
					}
					if ( mLocalTime == 0.0f )
						mpEventIF->onAnimationEvent("AnimDone", VuJsonContainer::null);
				}
				else
				{
					if ( bHandleTimedEvents )
					{
						handleTimedEventsReverse(prevTime, 0.0f);
						handleTimedEventsReverse(mpAnimation->getTotalTime(), mLocalTime);
					}
					mpEventIF->onAnimationEvent("AnimLoop", VuJsonContainer::null);
				}
			}
		}
	}
}

//*****************************************************************************
void VuAnimationControl::setLocalTime(float localTime)
{
	mLocalTime = localTime;
	advance(0);
}

//*****************************************************************************
bool VuAnimationControl::isAtStart() const
{
	return mLocalTime == 0.0f;
}

//*****************************************************************************
bool VuAnimationControl::isAtEnd() const
{
	return mLocalTime == mpAnimation->getEndTime();
}

//*****************************************************************************
void VuAnimationControl::setTimedEventAsset(VuTimedEventAsset *pTimedEventAsset)
{
	if ( mpTimedEventAsset )
		VuAssetFactory::IF()->releaseAsset(mpTimedEventAsset);

	mpTimedEventAsset = pTimedEventAsset;

	if ( mpTimedEventAsset )
		VuAssetFactory::IF()->addAssetRef(mpTimedEventAsset);
}

//*****************************************************************************
void VuAnimationControl::handleTimedEventsForward(float prevTime, float curTime)
{
	for ( int i = 0; i < mpTimedEventAsset->getEventCount(); i++ )
	{
		const VuTimedEventAsset::VuEvent *pEvent = &mpTimedEventAsset->getEvent(i);
		if ( pEvent->mTime >= prevTime && pEvent->mTime < curTime )
		{
			mpEventIF->onAnimationEvent(pEvent->mType, pEvent->mParams);
		}
		pEvent++;
	}
}

//*****************************************************************************
void VuAnimationControl::handleTimedEventsReverse(float prevTime, float curTime)
{
	for ( int i = mpTimedEventAsset->getEventCount() - 1; i >= 0; i-- )
	{
		const VuTimedEventAsset::VuEvent *pEvent = &mpTimedEventAsset->getEvent(i);
		if ( pEvent->mTime < prevTime && pEvent->mTime >= curTime )
		{
			mpEventIF->onAnimationEvent(pEvent->mType, pEvent->mParams);
		}
		pEvent++;
	}
}
