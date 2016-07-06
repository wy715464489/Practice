//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Color class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector4.h"


class VuColor
{
public:
	VuColor()													{}
	VuColor(VUUINT8 r, VUUINT8 g, VUUINT8 b, VUUINT8 a)			{ mR = r; mG = g; mB = b; mA = a; }
	VuColor(VUUINT8 r, VUUINT8 g, VUUINT8 b)					{ mR = r; mG = g; mB = b; mA = 255; }
	//explicit VuColor(float &r, float &g, float &b)				{ fromFloat3(r, g, b); }
	//explicit VuColor(float &r, float &g, float &b, float &a)	{ fromFloat4(r, g, b, a); }
	explicit VuColor(const VuVector3 &v)						{ fromVector3(v); }
	explicit VuColor(const VuVector4 &v)						{ fromVector4(v); }

	void	set(VUUINT8 r, VUUINT8 g, VUUINT8 b, VUUINT8 a)	{ mR = r; mG = g; mB = b; mA = a; }
	void	set(VUUINT8 r, VUUINT8 g, VUUINT8 b)			{ mR = r; mG = g; mB = b; mA = 255; }

#if VU_LITTLE_ENDIAN
	VUUINT8 mR, mG, mB, mA;
#elif VU_BIG_ENDIAN
	VUUINT8 mA, mB, mG, mR;
#else
		#error Endianness not defined!
#endif

	bool operator == (const VuColor &color) const { return *(VUUINT32 *)&color == *(VUUINT32 *)this; }
	bool operator != (const VuColor &color) const { return *(VUUINT32 *)&color != *(VUUINT32 *)this; }

	operator VUUINT32 () const	{ return *(VUUINT32 *)this; }

	void operator *=(const VuColor &col)	{ mR = (mR*col.mR)>>8; mG = (mG*col.mG)>>8; mB = (mB*col.mB)>>8; mA = (mA*col.mA)>>8; }
	void operator *=(float f)				{ mR = VUUINT8(mR*f + 0.5f); mG = VUUINT8(mG*f + 0.5f); mB = VUUINT8(mB*f + 0.5f); mA = VUUINT8(mA*f + 0.5f); }

	void toFloat3(float &r, float &g, float &b) const			{ r = mR/255.0f; g = mG/255.0f; b = mB/255.0f; }
	void toFloat4(float &r, float &g, float &b, float &a) const	{ r = mR/255.0f; g = mG/255.0f; b = mB/255.0f; a = mA/255.0f; }
	void toVector3(VuVector3 &v) const							{ toFloat3(v.mX, v.mY, v.mZ); }
	void toVector4(VuVector4 &v) const							{ toFloat4(v.mX, v.mY, v.mZ, v.mW); }
	VuVector3 toVector3() const									{ VuVector3 v; toFloat3(v.mX, v.mY, v.mZ); return v; }
	VuVector4 toVector4() const									{ VuVector4 v; toFloat4(v.mX, v.mY, v.mZ, v.mW); return v; }

	void fromFloat3(float r, float g, float b) 			{ mR = VUUINT8(r*255.0f + 0.5f); mG = VUUINT8(g*255.0f); mB = VUUINT8(b*255.0f + 0.5f); mA = 255; }
	void fromFloat4(float r, float g, float b, float a)	{ mR = VUUINT8(r*255.0f + 0.5f); mG = VUUINT8(g*255.0f); mB = VUUINT8(b*255.0f + 0.5f); mA = VUUINT8(a*255.0f + 0.5f); }
	void fromVector3(const VuVector3 &v)				{ fromFloat3(v.mX, v.mY, v.mZ); }
	void fromVector4(const VuVector4 &v)				{ fromFloat4(v.mX, v.mY, v.mZ, v.mW); }
};

inline VuColor operator *(const VuColor &c1, const VuColor &c2) { return VuColor((c1.mR*c2.mR)>>8, (c1.mG*c2.mG)>>8, (c1.mB*c2.mB)>>8, (c1.mA*c2.mA)>>8); }
inline VuColor operator *(const VuColor &c, float f) { return VuColor(VUUINT8(c.mR*f + 0.5f), VUUINT8(c.mG*f + 0.5f), VUUINT8(c.mB*f + 0.5f), VUUINT8(c.mA*f + 0.5f)); }

inline VuColor VuLerp(const VuColor &a, const VuColor &b, float factor)
{
	float ratioA = 1.0f - factor;
	float ratioB = factor;

	VuColor col;

	col.mR = VUUINT8(ratioA*a.mR + ratioB*b.mR + 0.5f);
	col.mG = VUUINT8(ratioA*a.mG + ratioB*b.mG + 0.5f);
	col.mB = VUUINT8(ratioA*a.mB + ratioB*b.mB + 0.5f);
	col.mA = VUUINT8(ratioA*a.mA + ratioB*b.mA + 0.5f);

	return col;
}
