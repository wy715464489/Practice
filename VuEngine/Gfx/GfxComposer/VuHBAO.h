//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  HBAO implementation class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/3dDraw/Vu3dDrawManager.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Containers/VuArray.h"

class VuShaderProgram;
class VuTexture;
class VuDepthRenderTarget;
class VuFxRenderTarget;
class VuGfxSortMaterial;
class VuPipelineState;
class VuTexture;
class VuJsonContainer;
class VuGfxSettings;

struct VuHBAOConsts
{
	VuVector4 mRadiusParams;	// radius scale, radius square, inv radius neg, max radius pixels;
	VuVector4 mBiasParams;		// angle bias, tanAngleBias, strength
	VuVector4 mScreenParams;	// aoResX, aoRexY, invAoRexX, invAoResY;
	VuVector4 mUvToViewParams;	// uvToViewA[0], uvToViewA[1], uvToViewB[0], uvToViewB[1]
	VuVector4 mFocalParams;
};


class VuHBAO
{
public:
	VuHBAO();
	~VuHBAO();

	bool				isEnabled() { return mEnabled; }
	void				configure(bool enabled, int width, int height);

	void				submitCommands();

	VuTexture			*getTexture();
	VuTexture			*getDepthTexture();
	VuTexture			*getNoiseTexture();

	struct VuMatExt
	{
		VUHANDLE		mhConstConstScreenSize;
		VUHANDLE		mhConstSSAOConsts;
		int				miSampSSAOTexture;
	};
	static void			resolveConstants(VuShaderProgram *pSP, VuMatExt *pMatExt);
	void				setConstants(VuShaderProgram *pSP, const VuMatExt *pMatExt);

private:
	static VuTexture	*createNoiseTexture();
	void				destroyResources();

	void				submitDepthCommands();
	void				submitEffectCommands();

	static void			calcHBAOConsts(int inputWidth, int inputHeight, float fovY, const VuGfxSettings &gfxSettings, VuHBAOConsts &consts);


	bool					mEnabled;
	int						mWidth;
	int						mHeight;
	VuDepthRenderTarget		*mpDepthRenderTarget;
	VuFxRenderTarget		*mpRenderTarget0;
	VuFxRenderTarget		*mpRenderTarget1;
	VuPipelineState			*mpHBAOPipelineState;
	VuPipelineState			*mpBlurPipelineState;
	VuTexture				*mpNoiseTexture;

	// HBAO effect
	VUHANDLE				mhConstRadius;
	VUHANDLE				mhConstBias;
	VUHANDLE				mhConstScreen;
	VUHANDLE				mhConstUvToView;
	VUHANDLE				mhConstFocal;
	VUHANDLE				mhConstNearFarPlanes;
	int						miSampDepthTexture;
	int						miSampNoiseTexture;

	// blur effect
	VUHANDLE				mhConstBlurFactors;
};
