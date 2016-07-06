//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Unit conversion
// 
//*****************************************************************************

#pragma once

#include "VuMath.h"


inline float VuHorsepowerToWatts(float f)	{ return f*745.69987158227022f; }
inline float VuWattsToHorsepower(float f)	{ return f*0.0013410220895950279f; }

inline float VuMphToMetersPerSecond(float f)	{ return f*0.44704f; }
inline float VuMphToKph(float f)				{ return f*1.609344f; }
inline float VuMetersPerSecondToMph(float f)	{ return f*2.23693629205f; }
inline float VuKphToMph(float f)				{ return f*0.621371192f; }

inline float VuFeetToMeters(float f)	{ return f*0.3048f; }
inline float VuMetersToFeet(float f)	{ return f*3.280839895013123f; }

inline float VuDecibelsToRatio(float db)	{ return VuPow(10.0f, 0.05f*db); }
inline float VuRatioToDecibels(float ratio)	{ return 20.0f*VuLog10(ratio); }

inline float VuFtlbToNm(float f)	{ return f*1.35581795f; }
inline float VuNmToFtlb(float f)	{ return f*0.737562148f; }
