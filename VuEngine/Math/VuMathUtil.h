//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MathUtil functions
// 
//*****************************************************************************

#pragma once

#include "VuVector2.h"
#include "VuVector3.h"
#include "VuVector4.h"

class VuVector2;
class VuMatrix;
class VuRect;
class VuQuaternion;
class VuAabb;


namespace VuMathUtil
{
	// Finds the point on line segment A-B closest to P.
	VuVector2 closestPointOnSegment(const VuVector2 &A, const VuVector2 &B, const VuVector2 &P);

	// Tests for an intersection between a line segment A-B and the rect R
	bool testIntersectionLineSegmentRect(const VuVector2 &A, const VuVector2 &B, const VuRect &R);

	// Tests for an intersection between a line segment A-B and the box defined by an aabb and transform
	bool testIntersectionLineSegBox(const VuVector3 &A, const VuVector3 &B, const VuAabb &aabb, const VuMatrix &transform);

	// returns the angle between two lines
	float angleLineLine(
		const VuVector3 &LA0,
		const VuVector3 &LA1,
		const VuVector3 &LB0,
		const VuVector3 &LB1);

	// returns the angle between two lines
	// the sign is determined by comparing the cross
	// product (AxB) with the normal N
	float signedAngleLineLine(
		const VuVector3 &LA0,
		const VuVector3 &LA1,
		const VuVector3 &LB0,
		const VuVector3 &LB1,
		const VuVector3 &N);

	// returns the distance between a point P and a line segment S0->S1
	// (and optionally the projection along the segment)
	float distPointLineSeg(
		const VuVector2 &P,
		const VuVector2 &S0,
		const VuVector2 &S1,
		float *pProjA = VUNULL);

	// returns the distance between a point P and a line segment S0->S1
	// (and optionally the projection along the segment)
	float distPointLineSeg(
		const VuVector3 &P,
		const VuVector3 &S0,
		const VuVector3 &S1,
		float *pProjA = VUNULL);

	// returns the point on line segment S0->S1 which is closest to point P
	void closestPointLineSeg(
		const VuVector3 &P,
		const VuVector3 &S0,
		const VuVector3 &S1,
		VuVector3 &closest);

	// returns the distance between a point P and a triagle A,B,C
	float distPointTriangle(
		const VuVector3 &P,
		const VuVector3 &A,
		const VuVector3 &B,
		const VuVector3 &C);

	// returns the distance between a point P and a box defined by an aabb and xform
	float distPointBox(const VuVector3 &P, const VuAabb &aabb, const VuMatrix &xform, VuVector3 &closestPoint);

	// returns the distance between two lines (and optionally the projection along each)
	float distLineLine(
		const VuVector3 &LA0,
		const VuVector3 &LA1,
		const VuVector3 &LB0,
		const VuVector3 &LB1,
		float *pProjA = VUNULL,
		float *pProjB = VUNULL);

	// returns the intersection of a plane defined by point P and normal N
	// and a line segment
	bool planeLineSegIntersection(
		const VuVector3 &P,
		const VuVector3 &N,
		const VuVector3 &S0,
		const VuVector3 &S1,
		VuVector3 &inter);

	// Finds the intersection of a line segment with the front of a sphere defined
	// by center C and radius R.
	bool sphereLineSegIntersection(
		const VuVector3 &C,
		float R,
		const VuVector3 &S0,
		const VuVector3 &S1,
		VuVector3 &inter);

	// Finds the intersection of a line segment with the triangle defined
	// by points A, B, C
	bool triangleLineSegIntersection(
		const VuVector3 &A,
		const VuVector3 &B,
		const VuVector3 &C,
		const VuVector3 &S0,
		const VuVector3 &S1,
		VuVector3 &inter);

	// Finds the intersection of two 2d line segments, if it exists
	bool lineSegLineSegIntersection2d(const VuVector2 &a0,
									  const VuVector2 &a1, 
									  const VuVector2 &b0, 
									  const VuVector2 &b1, 
									  VuVector2 &hitPos);

	// Based on previous and current positions, did the point pass a line 
	// segment defined as passing through "vLineCenter" and "width" which 
	// is perpendicular to "vNormal"
	bool crossedWidthBoundedPlane(const VuVector3 &vCurPos, 
								 const VuVector3 &vPrevPos, 
								 const VuVector3 &vLineCenter, 
								 const VuVector3 &vNormal, 
								 float width,
								 bool &forward);

	// conversion between spherical and cartesian coordinates
	inline VuVector3	sphericalToCartesian(const VuVector3 &vSpherical);
	inline VuVector3	cartesianToSpherical(const VuVector3 &vCartesian);

	// conversion between polar and cartesian coordinates
	inline VuVector2	polarToCartesian(const VuVector2 &vPolar);
	inline VuVector2	cartesianToPolar(const VuVector2 &vCartesian);

	// plane math
	inline VuVector4	planeFromNormalPoint(const VuVector3 &normal, const VuVector3 &point);
	inline VuVector4	planeFromVerts(const VuVector3 &v0, const VuVector3 &v1, const VuVector3 &v2);
	inline float		distPointPlane(const VuVector3 &point, const VuVector4 &plane);

	// matrix utility
	void	buildOrientationMatrix(const VuVector3 &vFwd, const VuVector3 &vUp, VuMatrix &mat);
	void	buildOrientationMatrixUp(const VuVector3 &vFwd, const VuVector3 &vUp, VuMatrix &mat);
	void	buildOrthographicProjectionMatrix(const VuRect &rect, float fNear, float fFar, VuMatrix &mat);

	// calculates a spline interpolation control point, given 3 quaternions qn-1, qn, and qn+1
	inline VuQuaternion	splineQuaternion(const VuQuaternion &qnm1, const VuQuaternion &qn, const VuQuaternion &qnp1);

	// returns the quaternion needed to go from a to b
	inline VuQuaternion	rotationDelta(const VuQuaternion &a, const VuQuaternion &b);

	// returns the rotation needed to go from a to b
	inline VuVector3	rotationDelta(const VuVector3 &a, const VuVector3 &b);

	// conversion
	inline VUUINT16	floatToHalf(float f);

	// critical damping
	inline float smoothCD(float from, float to, float &vel, float smoothTime, float deltaTime);
	inline VuVector3 smoothCD(const VuVector3 &from, const VuVector3 &to, VuVector3 &vel, float smoothTime, float deltaTime);

	inline float chase(float curVal, float targetVal, float step);

	// interpolation
	inline float interpolateLinearCurve(float t, const VuVector2 *sortedValues, int count);

	// Catmull-Rom spline interpolation
	inline VuVector3 interpolateCatmullRom(float t, const VuVector3 &p0, const VuVector3 &p1, const VuVector3 &p2, const VuVector3 &p3);

	// apply safe zone to screen rect
	inline VuRect applySafeZone(const VuRect &rect, float safeZone);
	inline VuVector2 unapplySafeZone(const VuVector2 &pos, float safeZone);

	//******************************************************************************
	// CalcDampedSimpleHarmonicMotion
	// This function will update the supplied position and velocity values over
	// the given time according to the spring parameters.
	// - An angular frequency is given to control how fast the spring oscillates.
	// - A damping ratio is given to control how fast the motion decays.
	//     damping ratio > 1: over damped
	//     damping ratio = 1: critically damped
	//     damping ratio < 1: under damped
	//******************************************************************************
	void calcDampedSimpleHarmonicMotion(
		float *pPos,            // position value to update
		float *pVel,            // velocity value to update
		float equilibriumPos,   // position to approach
		float deltaTime,        // time to update over
		float angularFrequency, // angular frequency of motion
		float dampingRatio      // damping ratio of motion
	);

} // namespace VuMathUtil


#include "VuMathUtil.inl"
