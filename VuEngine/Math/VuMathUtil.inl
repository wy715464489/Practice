//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MathUtil inline functionality
// 
//*****************************************************************************

#include "VuQuaternion.h"
#include "VuRect.h"


//*****************************************************************************
// conversion between spherical and cartesian coordinates
//*****************************************************************************
VuVector3 VuMathUtil::sphericalToCartesian(const VuVector3 &vSpherical)
{
	float sinY, cosY, sinZ, cosZ;
	VuSinCos(vSpherical.mY, sinY, cosY);
	VuSinCos(vSpherical.mZ, sinZ, cosZ);

	VuVector3 vCartesian;
	vCartesian.mX = vSpherical.mX*cosY*sinZ;
	vCartesian.mY = vSpherical.mX*sinY*sinZ;
	vCartesian.mZ = vSpherical.mX*cosZ;

	return vCartesian;
}
VuVector3 VuMathUtil::cartesianToSpherical(const VuVector3 &vCartesian)
{
	VuVector3 vSpherical;
	vSpherical.mX = vCartesian.mag();
	vSpherical.mY = VuATan2(vCartesian.mY, vCartesian.mX);
	vSpherical.mZ = VuACos(vCartesian.mZ/vSpherical.mX);

	return vSpherical;
}

//*****************************************************************************
// conversion between polar and cartesian coordinates
//*****************************************************************************
VuVector2 VuMathUtil::polarToCartesian(const VuVector2 &vPolar)
{
	float s, c;
	VuSinCos(vPolar.mY, s, c);

	VuVector2 vCartesian;
	vCartesian.mX = vPolar.mX*c;
	vCartesian.mY = vPolar.mX*s;

	return vCartesian;
}
VuVector2 VuMathUtil::cartesianToPolar(const VuVector2 &vCartesian)
{
	VuVector2 vPolar;
	vPolar.mX = vCartesian.mag();
	vPolar.mY = VuATan2(vCartesian.mY, vCartesian.mX);

	return vPolar;
}

//*****************************************************************************
VuVector4 VuMathUtil::planeFromNormalPoint(const VuVector3 &normal, const VuVector3 &point)
{
	return VuVector4(normal.mX, normal.mY, normal.mZ, -VuDot(normal, point));
}

//*****************************************************************************
VuVector4 VuMathUtil::planeFromVerts(const VuVector3 &v0, const VuVector3 &v1, const VuVector3 &v2)
{
	return VuMathUtil::planeFromNormalPoint(VuCross(v1 - v0, v2 - v0).normal(), v0);
}

//*****************************************************************************
float VuMathUtil::distPointPlane(const VuVector3 &point, const VuVector4 &plane)
{
	return point.mX*plane.mX + point.mY*plane.mY + point.mZ*plane.mZ + plane.mW;
}

//*****************************************************************************
VuQuaternion VuMathUtil::splineQuaternion(const VuQuaternion &qnm1, const VuQuaternion &qn, const VuQuaternion &qnp1)
{
	VuQuaternion qni = qn;
	qni.mVec.mW = -qni.mVec.mW;

	return qn*((-0.25f*((qni*qnm1).log() + (qni*qnp1).log())).exp());
}

//*****************************************************************************
VuQuaternion VuMathUtil::rotationDelta(const VuQuaternion &a, const VuQuaternion &b)
{
	VuQuaternion aInv = a;
	aInv.invert();

	return aInv*b;
}

//*****************************************************************************
VuVector3 VuMathUtil::rotationDelta(const VuVector3 &a, const VuVector3 &b)
{
	return VuVector3(VuAngDiff(a.mX, b.mX), VuAngDiff(a.mY, b.mY), VuAngDiff(a.mZ, b.mZ));
}

//*****************************************************************************
VUUINT16 VuMathUtil::floatToHalf(float f)
{
	VUUINT32 i = *(VUUINT32 *)&f;

	int s =  (i >> 16) & 0x00008000;
	int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int m =   i        & 0x007fffff;

	if (e <= 0)
	{
		if (e < -10)
		{
			return 0;
		}
		m = (m | 0x00800000) >> (1 - e);

		if (m & 0x00001000)
			m += 0x00002000;

		return static_cast<VUUINT16>(s | (m >> 13));
	}
	else if (e == 0xff - (127 - 15))
	{
		if (m == 0) // Inf
		{
			return static_cast<VUUINT16>(s | 0x7c00);
		} 
		else    // NAN
		{
			m >>= 13;
			return static_cast<VUUINT16>(s | 0x7c00 | m | (m == 0));
		}
	}
	else
	{
		if (m & 0x00001000)
		{
			m += 0x00002000;

			if (m & 0x00800000)
			{
				m = 0;		// overflow in significand,
				e += 1;		// adjust exponent
			}
		}
		if (e > 30) // Overflow
		{
			return static_cast<VUUINT16>(s | 0x7c00);
		}

		return static_cast<VUUINT16>(s | (e << 10) | (m >> 13));
	}
}

//*****************************************************************************
float VuMathUtil::smoothCD(float from, float to, float &vel, float smoothTime, float deltaTime)
{
	float omega = 2.0f/smoothTime;
	float x = omega*deltaTime;
	float exp = 1.0f/(1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
	float change = from - to;
	float temp = (vel + omega*change)*deltaTime;
	vel = (vel - omega*temp)*exp;
	return to + (change + temp)*exp;
}

//*****************************************************************************
inline VuVector3 VuMathUtil::smoothCD(const VuVector3 &from, const VuVector3 &to, VuVector3 &vel, float smoothTime, float deltaTime)
{
	VuVector3 out;
	out.mX = smoothCD(from.mX, to.mX, vel.mX, smoothTime, deltaTime);
	out.mY = smoothCD(from.mY, to.mY, vel.mY, smoothTime, deltaTime);
	out.mZ = smoothCD(from.mZ, to.mZ, vel.mZ, smoothTime, deltaTime);
	return out;
}

//*****************************************************************************
inline float VuMathUtil::chase(float curVal, float targetVal, float step)
{
	if ( targetVal > curVal )
		return VuMin(curVal + step, targetVal);
	
	return VuMax(curVal - step, targetVal);
}

//*****************************************************************************
inline float VuMathUtil::interpolateLinearCurve(float t, const VuVector2 *sortedValues, int count)
{
	VUASSERT(count > 0, "VuMathUtil::interpolateLinearCurve() invalid params");

	if ( t <= sortedValues[0].mX )
		return sortedValues[0].mY;

	for ( int i = 1; i < count; i++ )
	{
		if ( t < sortedValues[i].mX )
		{
			float t0 = sortedValues[i-1].mX;
			float t1 = sortedValues[i].mX;
			float ratio = (t - t0)/(t1 - t0);

			float val0 = sortedValues[i-1].mY;
			float val1 = sortedValues[i].mY;

			return VuLerp(val0, val1, ratio);
		}
	}

	return sortedValues[count - 1].mY;
}

//*****************************************************************************
inline VuVector3 VuMathUtil::interpolateCatmullRom(float t, const VuVector3 &p0, const VuVector3 &p1, const VuVector3 &p2, const VuVector3 &p3)
{
	return 0.5f*((2.0f*p1) + (-p0 + p2)*t + (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3)*(t*t) + (-p0 + 3.0f*p1- 3.0f*p2 + p3)*(t*t*t));
}

//*****************************************************************************
inline VuRect VuMathUtil::applySafeZone(const VuRect &rect, float safeZone)
{
	return ((rect - VuVector2(0.5f, 0.5f))*VuVector2(safeZone, safeZone)) + VuVector2(0.5f, 0.5f);
}

//*****************************************************************************
inline VuVector2 VuMathUtil::unapplySafeZone(const VuVector2 &pos, float safeZone)
{
	return ((pos - VuVector2(0.5f, 0.5f))/VuVector2(safeZone, safeZone)) + VuVector2(0.5f, 0.5f);
}
