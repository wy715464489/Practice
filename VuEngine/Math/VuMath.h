//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Math functions
// 
//*****************************************************************************

#pragma once

#include <float.h>
#include <math.h>


#define VU_PI               3.141592654f
#define VU_2PI              6.283185307f
#define VU_1DIVPI           0.318309886f
#define VU_1DIV2PI          0.159154943f
#define VU_PIDIV2           1.570796327f
#define VU_PIDIV4           0.785398163f

// int min/max/clamp/abs/select
inline int VuMin(int a, int b)								{ return a < b ? a : b; }
inline VUUINT VuMin(VUUINT a, VUUINT b)						{ return a < b ? a : b; }
inline int VuMax(int a, int b)								{ return a > b ? a : b; }
inline VUUINT VuMax(VUUINT a, VUUINT b)						{ return a > b ? a : b; }
inline int VuClamp(int val, int min, int max)				{ return VuMax(min, VuMin(max, val)); }
inline VUUINT VuClamp(VUUINT val, VUUINT min, VUUINT max)	{ return VuMax(min, VuMin(max, val)); }
inline void VuMinMax(int val, int &min, int &max)			{ min = VuMin(val, min); max = VuMax(val, max); }
inline void VuMinMax(VUUINT val, VUUINT &min, VUUINT &max)	{ min = VuMin(val, min); max = VuMax(val, max); }
inline int VuAbs(VUINT32 i)									{ return (i^(i>>31)) - (i>>31); }
inline int VuSelect(VUINT condition, VUINT valueNonZero, VUINT valueZero)
{
    VUINT testNz = ((condition | -condition) >> 31);
    VUINT testEqz = ~testNz; 
    return (valueNonZero & testNz) | (valueZero & testEqz);
}

// float min/max/clamp/abs/select
inline float VuMin(float a, float b);
inline float VuMax(float a, float b);
inline float VuClamp(float val, float min, float max)	{ return VuMax(min, VuMin(max, val)); }
inline float VuAbs(float f)								{ return fabsf(f); }
inline double VuAbs(double d)							{ return fabs(d); }
inline void VuMinMax(float val, float &min, float &max)	{ min = VuMin(val, min); max = VuMax(val, max); }
inline float VuSelect(float condition, float valueGreaterEqualZero, float valueLessZero);

inline float VuRadiansToDegrees(float f)	{ return f*57.295779513082320876798154814105f; }
inline float VuDegreesToRadians(float f)	{ return f*0.017453292519943295769236907684886f; }

inline float VuSqrt(float f)			{ return sqrtf(f); }
inline float VuSquare(float f)			{ return f*f; }
inline float VuExp(float f)				{ return expf(f); }
inline float VuLog(float f)				{ return logf(f); }
inline float VuLog10(float f)			{ return log10f(f); }
inline float VuPow(float x, float y)	{ return powf(x,y); }
inline float VuModf(float x, float y)	{ return fmodf(x,y); }

inline float VuCos(float f);
inline float VuSin(float f);
inline void VuSinCos(float f, float &fs, float &fc);
inline float VuCosEst(float f);
inline float VuSinEst(float f);
inline void VuSinCosEst(float f, float &fs, float &fc);
inline float VuModAngle(float f); // computes an angle between -VU_PI and VU_PI

inline float VuACos(float f)						{ VUASSERT(VuAbs(f) <= 1, "VuACos() invalid param"); return acosf(f); }
inline float VuASin(float f)						{ VUASSERT(VuAbs(f) <= 1, "VuASin() invalid param"); return asinf(f); }
inline float VuTan(float f)							{ return tanf(f); }
inline float VuATan(float f)						{ return atanf(f); }
inline float VuATan2(float y, float x)				{ return atan2f(y, x); }

inline float VuLerp(float val0, float val1, float factor)	{ return (1.0f - factor)*val0 + factor*val1; }
inline float VuSmoothStep(float val0, float val1, float x)
{
	if ( x < val0 ) return 0.0f;
	if ( x >= val1 ) return 1.0f;
	float temp = (x - val0)/(val1 - val0);
	return 3.0f*temp*temp - 2.0f*temp*temp*temp;
}
float VuAngClamp(float ang);
float VuAngDiff(float ang0, float ang1);
float VuAngLerp(float ang0, float ang1, float factor);

inline float VuLinStep(float range0, float range1, float val)
{
	if ( val <= range0 ) return 0.0f;
	if ( val >= range1 ) return 1.0f;
	return (val - range0)/(range1 - range0);
}

inline VUINT VuTruncate(float f);
inline VUINT VuRound(float f);
inline VUINT VuFloorInt(float f);
inline VUINT VuCeilInt(float f);
inline float VuFloor(float f);
inline float VuCeil(float f);

inline float VuRound(float f, float precision)
{
	return VuRound(f/precision)*precision;
}

inline int VuIsFinite(float f);


#if defined (VUWIN32)
	#include "Win32/VuWin32Math.inl"
#elif defined (VUXBOX360)
	#include "Xbox360/VuXbox360Math.inl"
#elif defined (VUANDROID)
	#include "Android/VuAndroidMath.inl"
#elif defined (VUIOS)
	#include "Ios/VuIosMath.inl"
#elif defined (VUWINSTORE) || defined (VUWINPHONE)
	#if defined (_M_ARM)
		#include "Windows/VuWindowsMathARM.inl"
	#else
		#include "Windows/VuWindowsMathX86.inl"
	#endif
#elif defined (VUXB1)
	#include "Xb1/VuXb1Math.inl"
#else
	#include "Generic/VuGenericMath.inl"
#endif
