//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios accelerometer hardware abstration layer
//
//*****************************************************************************

#include "VuIosAccel.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuAccel, VuIosAccel);


//*****************************************************************************
VuIosAccel::VuIosAccel():
	mHistoryIndex(0),
	mAccel(0,0,0)
{
	memset(mHistory, 0, sizeof(mHistory));
}

//*****************************************************************************
void VuIosAccel::onAccel(float accX, float accY, float accZ)
{
	// calculate raw acceleration
	VuVector3 screenAccel = VuVector3(accX, accY, accZ);

	// add to history
	mHistory[mHistoryIndex] = screenAccel;
	mHistoryIndex = (mHistoryIndex + 1)%HISTORY_SIZE;

	// determine high and low values
	VuVector3 high = mHistory[0];
	VuVector3 low = mHistory[0];
	for ( int i = 1; i < HISTORY_SIZE; i++ )
		VuMinMax(mHistory[i], low, high);

	// calc average, disregarding high and low values
	VuVector3 total = VuVector3(0,0,0);
	for ( int i = 0; i < HISTORY_SIZE; i++ )
		total += mHistory[i];
	mAccel = (total - high - low)/(HISTORY_SIZE - 2);
}
