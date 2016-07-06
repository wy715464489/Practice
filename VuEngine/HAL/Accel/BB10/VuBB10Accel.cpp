//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  BB10 accelerometer hardware abstration layer
//
//*****************************************************************************

#include "VuBB10Accel.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAccel, VuBB10Accel);

#define MAX_HISTORY_SIZE 10
#define MAX_HISTORY_TIME_MS 150


//*****************************************************************************
VuBB10Accel::VuBB10Accel():
	mAccel(0,0,0),
	mIsActive(false)
{
}

//*****************************************************************************
void VuBB10Accel::onAccel(const VuVector3 &rawAccel)
{
	// calculate raw acceleration
	VuVector3 accel = rawAccel/9.806f;

	VUUINT32 curTimeMS = VuSys::IF()->getTimeMS();

	// remove old values from history
	while ( mHistory.size() && (curTimeMS - mHistory[0].mTimeMS > MAX_HISTORY_TIME_MS) || mHistory.size() >= MAX_HISTORY_SIZE )
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
