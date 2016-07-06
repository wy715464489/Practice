//*****************************************************************************
//
//  Copyright (c) 2009-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  WinStore math functions
// 
//*****************************************************************************


inline float VuMin(float a, float b)					{ return a < b ? a : b; }
inline float VuMax(float a, float b)					{ return a > b ? a : b; }
inline float VuSelect(float a, float b, float c)		{ return a >= 0 ? b : c; }

inline float VuCos(float f)                            { return DirectX::XMScalarCos(f); }
inline float VuSin(float f)                            { return DirectX::XMScalarSin(f); }
inline void VuSinCos(float f, float &fs, float &fc)    { DirectX::XMScalarSinCos(&fs, &fc, f); }
inline float VuCosEst(float f)                         { return DirectX::XMScalarCosEst(f); }
inline float VuSinEst(float f)                         { return DirectX::XMScalarSinEst(f); }
inline void VuSinCosEst(float f, float &fs, float &fc) { DirectX::XMScalarSinCosEst(&fs, &fc, f); }
inline float VuModAngle(float f)                       { return DirectX::XMScalarModAngle(f); }

inline VUINT VuTruncate(float f)	{ return (VUINT)f; }
inline VUINT VuRound(float f)		{ return (VUINT)floor(f + 0.5f); }
inline VUINT VuFloorInt(float f)	{ return (VUINT)floor(f); }
inline VUINT VuCeilInt(float f)		{ return (VUINT)ceil(f); }
inline float VuFloor(float f)		{ return floor(f); }
inline float VuCeil(float f)		{ return ceil(f); }
inline int VuIsFinite(float f)		{ return isfinite(f); }