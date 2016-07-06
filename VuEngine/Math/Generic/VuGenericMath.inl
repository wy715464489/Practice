//*****************************************************************************
//
//  Copyright (c) 2009-2009 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic math functions
// 
//*****************************************************************************


inline float VuMin(float a, float b)					{ return a < b ? a : b; }
inline float VuMax(float a, float b)					{ return a > b ? a : b; }
inline float VuSelect(float a, float b, float c)		{ return a >= 0 ? b : c; }

inline float VuCos(float f)								{ return cosf(f); }
inline float VuSin(float f)								{ return sinf(f); }
inline void VuSinCos(float f, float &fs, float &fc)		{ fs = sinf(f); fc = cosf(f); }
inline float VuCosEst(float f)							{ return VuCos(f); }
inline float VuSinEst(float f)							{ return VuSin(f); }
inline void VuSinCosEst(float f, float &fs, float &fc)	{ VuSinCos(f, fs, fc); }
inline float VuModAngle(float f)
{
	f = f + VU_PI;				// Normalize the range from 0.0f to VU_2PI
	float fTemp = VuAbs(f);		// Perform the modulo, unsigned
	fTemp = fTemp - (VU_2PI*(float)((VUINT)(fTemp/VU_2PI)));
	fTemp = fTemp - VU_PI;		// Restore the number to the range of -VU_PI to VU_PI-epsilon
	if ( f < 0.0f )				// If the modulo'd value was negative, restore negation
		fTemp = -fTemp;
	return fTemp;
}

inline VUINT VuTruncate(float f)	{ return (VUINT)f; }
inline VUINT VuRound(float f)		{ return (VUINT)(f > 0.0f ? f + 0.5f : f - 0.5f); }
inline VUINT VuFloorInt(float f)	{ return (VUINT)floor(f); }
inline VUINT VuCeilInt(float f)		{ return (VUINT)ceil(f); }
inline float VuFloor(float f)		{ return floor(f); }
inline float VuCeil(float f)		{ return ceil(f); }
inline int VuIsFinite(float f)		{ return isfinite(f); }