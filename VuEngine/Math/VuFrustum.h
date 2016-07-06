//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Frustum class
// 
//*****************************************************************************

#pragma once

#include "VuVector3.h"

class VuAabb;
class VuMatrix;
class VuVector4;


class VuFrustum
{
public:
	VuFrustum();
	VuFrustum(const VuVector3 &vO, const VuVector3 &vD, const VuVector3 &vU, const VuVector3 &vR,
	          float fDMin, float fDMax, float fUBound, float fRBound);

	void		update();

	void		getVerts(VuVector3 *pVerts) const;		// 8 verts
	void		getPlanes(VuVector4 *pPlanes) const;	// 6 planes (near/far/left/right/top/bottom)

	bool		isAabbVisible(const VuAabb &aabb, const VuMatrix &matWorld) const;
	

	VuVector3	mO;			// origin
	VuVector3	mD;			// direction vector
	VuVector3	mU;			// up vector
	VuVector3	mR;			// right vector
	float		mDMin;		// near distance
	float		mDMax;		// far distance
	float		mUBound;	// extent in U direction at near plane
	float		mRBound;	// extent in R direction at near plane

protected:
	// derived quantities
	float		mDRatio;
	float		mMTwoUF;
	float		mMTwoRF;
};
