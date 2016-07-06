//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Collision Shader class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Util/VuColor.h"

class VuMatrix;
class VuGfxSortMaterial;


class VuCollisionShader
{
public:
	VuCollisionShader();

	bool				init();
	void				release();

	VuGfxSortMaterial	*getMaterial();
	void				setConstants(const VuMatrix &modelMatrix, const VuColor &color = VuColor(255,255,255));

	struct VuVertex
	{
		float		mXyz[3];
		VUUINT32	mColor;
	};

private:
	VuGfxSortMaterial	*mpMaterial;

	VUHANDLE			mCollisionModelMatrixHandle;
	VUHANDLE			mCollisionDiffuseColorHandle;
};
