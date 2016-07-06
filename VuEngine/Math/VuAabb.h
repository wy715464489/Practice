//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Aabb class
// 
//*****************************************************************************

#pragma once

#include <float.h>

#include "VuVector3.h"

class VuMatrix;
class VuPackedVector3;


class VuAabb
{
public:
	VuAabb() { reset(); }
	VuAabb(const VuVector3 &vMin, const VuVector3 &vMax) : mMin(vMin), mMax(vMax) {}
	VuAabb(const VuAabb &aabb, const VuMatrix &mat) { reset(); addAabb(aabb, mat); }
	VuAabb(const VuVector3 &vPos) : mMin(vPos), mMax(vPos) {}
	VuAabb(const VuVector3 &vPos, float radius)
	{
		mMin.mX = vPos.mX - radius; mMin.mY = vPos.mY - radius; mMin.mZ = vPos.mZ - radius;
		mMax.mX = vPos.mX + radius; mMax.mY = vPos.mY + radius; mMax.mZ = vPos.mZ + radius;
	}

	void			reset() { mMin = VuVector3(FLT_MAX, FLT_MAX, FLT_MAX); mMax = VuVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX); }

	void			addPoint(const VuVector3 &vPos) { VuMinMax(vPos, mMin, mMax); }
	void			addAabb(const VuAabb &aabb);
	void			addAabb(const VuAabb &aabb, const VuMatrix &mat);
	void			addSphere(const VuVector3 &vPos, float radius);

	bool			isValid() const		{ return mMin.mX != FLT_MAX; }
	VuVector3		getSize() const		{ return mMax - mMin; }
	VuVector3		getCenter() const	{ return 0.5f*(mMin + mMax); }
	VuVector3		getExtents() const	{ return 0.5f*(mMax - mMin); }
	float			getVolume() const	{ return (mMax.mX - mMin.mX)*(mMax.mY - mMin.mY)*(mMax.mZ - mMin.mZ); }

	void			getVerts(VuVector3 *pVerts) const;			// 8 verts
	void			getVerts(VuPackedVector3 *pVerts) const;	// 8 verts
	const VUUINT16	*getEdgeIndices() const;					// 24 indices
	const VUUINT16	*getTriIndices() const;						// 36 indices

	void			getPosNorVerts(VuPackedVector3 *pVerts) const;	// 24 pos/nor pairs (48 packed vector3s)
	const VUUINT16	*getPosNorTriIndices() const;					// 36 indices

	// containment/intersection
	inline bool	contains(const VuAabb &bounds) const;
	inline bool	contains(const VuVector3 &point) const;
	inline bool	intersects(const VuAabb &bounds) const;

	VuVector3	mMin;
	VuVector3	mMax;

	static inline const VuAabb	&zero()	{ return smAabbZero; }
	static inline const VuAabb	&one()	{ return smAabbOne; }

private:
	static VuAabb	smAabbZero;
	static VuAabb	smAabbOne;
};

#include "VuAabb.inl"