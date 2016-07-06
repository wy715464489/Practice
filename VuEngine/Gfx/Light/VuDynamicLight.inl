//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DynamicLight inline implementation
// 
//*****************************************************************************

#include "VuEngine/Math/VuMatrix.h"


//*****************************************************************************
inline bool VuRenderLight::testBounds(const VuMatrix &transform, const VuAabb &aabb) const
{
	VuVector3 center = transform.transform(aabb.getCenter());
	VuVector3 extents = aabb.getExtents();

	VuVector3 vDiff;
	vDiff.mX = mPosition.mX - center.mX;
	vDiff.mY = mPosition.mY - center.mY;
	vDiff.mZ = mPosition.mZ - center.mZ;

	float distX = VuAbs(VuDot(vDiff, transform.getAxisX())) - extents.mX;
	float distY = VuAbs(VuDot(vDiff, transform.getAxisY())) - extents.mY;
	float distZ = VuAbs(VuDot(vDiff, transform.getAxisZ())) - extents.mZ;

	float maxDist = VuMax(VuMax(distX, distY), distZ);

	return maxDist < mRange.mY;
}

//*****************************************************************************
inline bool VuRenderLight::testBounds(const VuAabb &aabb) const
{
	VuVector3 center = aabb.getCenter();
	VuVector3 extents = aabb.getExtents();

	VuVector3 vDiff;
	vDiff.mX = mPosition.mX - center.mX;
	vDiff.mY = mPosition.mY - center.mY;
	vDiff.mZ = mPosition.mZ - center.mZ;

	float distX = VuAbs(vDiff.mX) - extents.mX;
	float distY = VuAbs(vDiff.mY) - extents.mY;
	float distZ = VuAbs(vDiff.mZ) - extents.mZ;

	float maxDist = VuMax(VuMax(distX, distY), distZ);

	return maxDist < mRange.mY;
}

//*****************************************************************************
inline bool VuRenderLight::testBounds(const VuVector3 &point) const
{
	VuVector3 vDiff;
	vDiff.mX = mPosition.mX - point.mX;
	vDiff.mY = mPosition.mY - point.mY;
	vDiff.mZ = mPosition.mZ - point.mZ;

	float distSquared = vDiff.magSquared();

	return distSquared < mRange.mY;
}
