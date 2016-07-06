//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Post-process techniques.
// 
//*****************************************************************************

#pragma once

class VuTexture;
class VuSurface;
class VuPipelineState;
class VuDepthStencilState;
class VuVector2;
class VuColor;


class VuPostProcess
{
public:
	VuPostProcess();

	bool	init();
	void	release();

	void	copy(VuTexture *pInputTexture);
	void	radialBlur(VuTexture *pInputTexture, float fAmount);
	void	colorCorrection(VuTexture *pInputTexture, const VuColor &contrast, const VuColor &tint, float gammaMin, float gammaMax, float gammaCurve);
	void	antiAlias(VuTexture *pInputTexture);
	void	gaussBlur(VuTexture *pInputTexture, float horzAmount, float vertAmount);

	void	drawFullScreenQuad();

private:
	static VuPipelineState	*createPostProcess(const char *shaderAsset);

	VuPipelineState		*mpCopyPipelineState;
	VuPipelineState		*mpGaussBlurPipelineState;
	VuPipelineState		*mpRadialBlurPipelineState;
	VuPipelineState		*mpTintContrastPipelineState;
	VuPipelineState		*mpTintContrastGammaPipelineState;
	VuPipelineState		*mpAntiAliasPipelineState;

	VUHANDLE			mhSpConstCopyTexelSize;

	VUHANDLE			mhSpConstGaussBlurTexelOffset;

	VUHANDLE			mhSpConstRadialBlurTexelSize;
	VUHANDLE			mhSpConstRadialBlurAmount;

	VUHANDLE			mhSpConstTintContrastTexelSize;
	VUHANDLE			mhSpConstTintContrastContrast;
	VUHANDLE			mhSpConstTintContrastTint;

	VUHANDLE			mhSpConstTintContrastGammaTexelSize;
	VUHANDLE			mhSpConstTintContrastGammaContrast;
	VUHANDLE			mhSpConstTintContrastGammaTint;
	VUHANDLE			mhSpConstTintContrastGammaMin;
	VUHANDLE			mhSpConstTintContrastGammaInvScale;
	VUHANDLE			mhSpConstTintContrastGammaCurve;

	VUHANDLE			mhSpConstAntiAliasTexelSize;
};
