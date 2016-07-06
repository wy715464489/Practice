//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuBlobShadowShader
// 
//*****************************************************************************

#pragma once

class VuPipelineState;


class VuBlobShadowShader
{
public:
	VuBlobShadowShader();

	bool				init();
	void				release();

	VuPipelineState		*getPipelineState() const { return mpPipelineState; }

private:
	VuPipelineState		*mpPipelineState;
};

class VuBlobShadowVertex
{
public:
	float		mXyz[3];
	float		mUv[2];
	VUUINT32	mColor;
};
