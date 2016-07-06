//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ShadowClip class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuVector4.h"

class VuVector3;
class VuMatrix;
class VuFrustum;
class VuAabb;


class VuShadowClip
{
public:
	VuShadowClip() : mClipPlaneCount(0) {}

	void		create(const VuVector3 &lightPos, const VuVector3 &lightDir, const VuFrustum &frustum);
	void		create(const VuMatrix &lightMatrix, const VuAabb &lightAabb);
	bool		isSphereVisible(const VuVector3 &vPosWorld, float fRadius) const;

	enum { MAX_CLIP_PLANES = 12 };

	VuVector4	mClipPlanes[MAX_CLIP_PLANES];
	int			mClipPlaneCount;
};
