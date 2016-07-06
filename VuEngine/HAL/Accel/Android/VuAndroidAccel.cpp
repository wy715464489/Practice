//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android accelerometer hardware abstration layer
//
//*****************************************************************************

#include "VuAndroidAccel.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Json/VuJsonContainer.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAccel, VuAndroidAccel);

#define MAX_HISTORY_SIZE 10
#define MAX_HISTORY_TIME_MS 150


//*****************************************************************************
VuAndroidAccel::VuAndroidAccel():
	mAccel(0,0,0),
	mIsActive(false)
{
}

//*****************************************************************************
void VuAndroidAccel::onGravityEvent(const VuVector3 &rawVector)
{
	mAccel = rawVector/9.806f;
}

//*****************************************************************************
void VuAndroidAccel::onAccelEvent(const VuVector3 &rawAccel)
{
	// calculate raw acceleration
	VuVector3 accel = rawAccel/9.806f;

	VUUINT32 curTimeMS = VuSys::IF()->getTimeMS();

	// remove old values from history
	while ( (mHistory.size() && (curTimeMS - mHistory[0].mTimeMS > MAX_HISTORY_TIME_MS)) || mHistory.size() >= MAX_HISTORY_SIZE )
		mHistory.erase(0);

	// add to history
	VuEntry entry;
	entry.mValue = accel;
	entry.mTimeMS = VuSys::IF()->getTimeMS();
	mHistory.push_back(entry);

	if ( mHistory.size() > 4 )
	{
		// determine high and low values
		VuVector3 high = mHistory[0].mValue;
		VuVector3 low = mHistory[0].mValue;
		for ( int i = 1; i < mHistory.size(); i++ )
			VuMinMax(mHistory[i].mValue, low, high);

		// calc average, disregarding high and low values
		VuVector3 total = VuVector3(0,0,0);
		for ( int i = 0; i < mHistory.size(); i++ )
			total += mHistory[i].mValue;
		mAccel = (total - high - low)/((float)mHistory.size() - 2);
	}
	else
	{
		// calc average, disregarding high and low values
		VuVector3 total = VuVector3(0,0,0);
		for ( int i = 0; i < mHistory.size(); i++ )
			total += mHistory[i].mValue;
		mAccel = total/(float)mHistory.size();
	}

	mIsActive = true;
}
