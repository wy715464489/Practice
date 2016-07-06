//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SSAO implementation class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/3dDraw/Vu3dDrawManager.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Containers/VuArray.h"

class VuShaderProgram;
class VuTexture;
class VuDepthRenderTarget;
class VuRenderTarget;
class VuGfxSortMaterial;
class VuPipelineState;
class VuTextureAsset;


class VuSSAO
{
public:
	VuSSAO();
	~VuSSAO();

	bool				isEnabled() { return mEnabled; }
	void				configure(bool enabled, int width, int height);

	void				submitCommands();

	VuTexture			*getDepthTexture();
	VuTexture			*getTexture();

	struct VuMatExt
	{
		VUHANDLE		mhConstConstScreenSize;
		int				miSampSSAOTexture;
	};
	static void			resolveConstants(VuShaderProgram *pSP, VuMatExt *pMatExt);
	void				setConstants(VuShaderProgram *pSP, const VuMatExt *pMatExt);

private:
	void				destroyResources();

	void				submitDepthCommands();
	void				submitEffectCommands();

	bool					mEnabled;
	int						mWidth;
	int						mHeight;
	VuDepthRenderTarget		*mpDepthRenderTarget;
	VuRenderTarget			*mpRenderTarget;
	VuPipelineState			*mpEffectPipelineState;
	VuTextureAsset			*mpNoiseTexture;

	VUHANDLE				mhSpConstTexelSize;
	VUHANDLE				mhSpConstScreenSize;
	VUHANDLE				mhSpConstNoiseSize;
	int						miSampNoiseTexture;
	int						miSampDepthTexture;
};
