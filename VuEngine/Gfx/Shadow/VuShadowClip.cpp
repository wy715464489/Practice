//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ShadowClip class
// 
//*****************************************************************************

#include "VuShadowClip.h"
#include "VuEngine/Math/VuFrustum.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
void VuShadowClip::create(const VuVector3 &lightPos, const VuVector3 &lightDir, const VuFrustum &frustum)
{
	#define ADD_PLANE(plane)																				\
	{																										\
		VUASSERT(mClipPlaneCount < MAX_CLIP_PLANES, "VuShadowClip::create(): exceeded max clip planes");	\
		mClipPlanes[mClipPlaneCount++] = plane;																\
	}

	// get light info
	VuVector4 lightDir4(lightDir.mX, lightDir.mY, lightDir.mZ, 0);

	// get frustum info
	VuVector3 verts[8];
	VuVector4 planes[6];
	frustum.getVerts(verts);
	frustum.getPlanes(planes);

	// calculate dot products between frustum planes and light direction
	float dots[6];
	for ( int i = 0; i < 6; i++ )
		dots[i] = VuDot(planes[i], lightDir4);

	// add light plane
	ADD_PLANE(VuMathUtil::planeFromNormalPoint(lightDir, lightPos));

	// add frustum planes facing towards light
	for ( int i = 0; i < 6; i++ )
		if ( dots[i] <= 0 )
			ADD_PLANE(planes[i]);

	// add silhouette planes
	struct Edge { int iv0; int iv1; int ip0; int ip1; };
	static Edge sEdges[] =
	{
		{ 0, 1, 0, 5 }, // near/bottom
		{ 1, 2, 0, 3 }, // near/right
		{ 2, 3, 0, 4 }, // near/top
		{ 3, 0, 0, 2 }, // near/left

		{ 4, 5, 1, 5 }, // far/bottom
		{ 5, 6, 1, 3 }, // far/right
		{ 6, 7, 1, 4 }, // far/top
		{ 7, 4, 1, 2 }, // far/left

		{ 0, 4, 2, 5 }, // left/bottom
		{ 1, 5, 5, 3 }, // bottom/right
		{ 2, 6, 3, 4 }, // right/top
		{ 3, 7, 4, 2 }, // top/left
	};

	//near/far/left/right/top/bottom
	VuVector3 center = frustum.mO + (0.5f*(frustum.mDMin + frustum.mDMax))*frustum.mD;
	for ( int i = 0; i < sizeof(sEdges)/sizeof(sEdges[0]); i++ )
	{
		Edge *pe = &sEdges[i];
		if ( dots[pe->ip0]*dots[pe->ip1] < 0 )
		{
			VuVector4 plane = VuMathUtil::planeFromNormalPoint(VuCross(verts[pe->iv0] - verts[pe->iv1], lightDir).normal(), verts[pe->iv0]);
			if ( VuMathUtil::distPointPlane(center, plane) < 0 )
				plane = -plane;

			ADD_PLANE(plane);
		}
	}
}

//*****************************************************************************
void VuShadowClip::create(const VuMatrix &lightMatrix, const VuAabb &lightAabb)
{
	VuMatrix lightTransform = lightMatrix;
	lightTransform.invert();

	// right/left
	mClipPlanes[0] = VuMathUtil::planeFromNormalPoint(-lightTransform.getAxisX(), lightTransform.getTrans() + lightAabb.mMax.mX*lightTransform.getAxisX());
	mClipPlanes[1] = VuMathUtil::planeFromNormalPoint( lightTransform.getAxisX(), lightTransform.getTrans() + lightAabb.mMin.mX*lightTransform.getAxisX());

	// top/bottom
	mClipPlanes[2] = VuMathUtil::planeFromNormalPoint(-lightTransform.getAxisY(), lightTransform.getTrans() + lightAabb.mMax.mY*lightTransform.getAxisY());
	mClipPlanes[3] = VuMathUtil::planeFromNormalPoint( lightTransform.getAxisY(), lightTransform.getTrans() + lightAabb.mMin.mY*lightTransform.getAxisY());

	// far/near
	mClipPlanes[4] = VuMathUtil::planeFromNormalPoint(-lightTransform.getAxisZ(), lightTransform.getTrans() + lightAabb.mMax.mZ*lightTransform.getAxisZ());
	mClipPlanes[5] = VuMathUtil::planeFromNormalPoint( lightTransform.getAxisZ(), lightTransform.getTrans() + lightAabb.mMin.mZ*lightTransform.getAxisZ());

	mClipPlaneCount = 6;
}

//*****************************************************************************
bool VuShadowClip::isSphereVisible(const VuVector3 &vPosWorld, float fRadius) const
{
	VUUINT32 fail = 0x0;

	for ( int i = 0; i < mClipPlaneCount; i++ )
	{
		float dist = VuMathUtil::distPointPlane(vPosWorld, mClipPlanes[i]);
		dist += fRadius;
		fail |= (int&)(dist)&0x80000000;
	}

	return !fail;
}
