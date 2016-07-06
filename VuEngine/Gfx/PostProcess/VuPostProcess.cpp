//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Post-process techniques.
// 
//*****************************************************************************

#include "VuPostProcess.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSortMaterial.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Math/VuVector2.h"

#define MAX_SAMPLES 16


// static functions

//*****************************************************************************
// Computes a two-parameter (x,y) Gaussian distrubution using the given
// standard deviation (rho)
//*****************************************************************************
static inline float gaussianDistribution(float x, float y, float rho)
{
    return expf( -( x * x + y * y ) / ( 2 * rho * rho ) ) / VuSqrt( 2 * VU_PI * rho * rho );
}

//*****************************************************************************
VuPostProcess::VuPostProcess()
{
}

//*****************************************************************************
VuPipelineState *VuPostProcess::createPostProcess(const char *shaderAsset)
{
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderAsset);

	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 0, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 8, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(16));
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

	VuPipelineStateParams psParams;
	VuPipelineState *pPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

	VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	pVD->removeRef();

	return pPipelineState;
}

//*****************************************************************************
bool VuPostProcess::init()
{
	// create shader programs

	// copy
	{
		mpCopyPipelineState = createPostProcess("PostProcess/Copy");
		VuShaderProgram *pSP = mpCopyPipelineState->mpShaderProgram;

		mhSpConstCopyTexelSize = pSP->getConstantByName("gTexelSize");
	}

	// gauss blur 5x5
	{
		mpGaussBlurPipelineState = createPostProcess("PostProcess/GaussBlur");
		VuShaderProgram *pSP = mpGaussBlurPipelineState->mpShaderProgram;

		mhSpConstGaussBlurTexelOffset = pSP->getConstantByName("gTexelOffset");
	}

	// radial blur
	{
		mpRadialBlurPipelineState = createPostProcess("PostProcess/RadialBlur");
		VuShaderProgram *pSP = mpRadialBlurPipelineState->mpShaderProgram;

		mhSpConstRadialBlurTexelSize = pSP->getConstantByName("gTexelSize");
		mhSpConstRadialBlurAmount = pSP->getConstantByName("gRadialBlurAmount");
	}

	// contrast/tint
	{
		mpTintContrastPipelineState = createPostProcess("PostProcess/TintContrast");
		VuShaderProgram *pSP = mpTintContrastPipelineState->mpShaderProgram;

		mhSpConstTintContrastTexelSize = pSP->getConstantByName("gTexelSize");
		mhSpConstTintContrastContrast = pSP->getConstantByName("gContrast");
		mhSpConstTintContrastTint = pSP->getConstantByName("gTint");
	}

	// contrast/tint/gamma
	{
		mpTintContrastGammaPipelineState = createPostProcess("PostProcess/TintContrastGamma");
		VuShaderProgram *pSP = mpTintContrastGammaPipelineState->mpShaderProgram;

		mhSpConstTintContrastGammaTexelSize = pSP->getConstantByName("gTexelSize");
		mhSpConstTintContrastGammaContrast = pSP->getConstantByName("gContrast");
		mhSpConstTintContrastGammaTint = pSP->getConstantByName("gTint");
		mhSpConstTintContrastGammaMin = pSP->getConstantByName("gGammaMin");
		mhSpConstTintContrastGammaInvScale = pSP->getConstantByName("gGammaInvScale");
		mhSpConstTintContrastGammaCurve = pSP->getConstantByName("gGammaCurve");
	}

	// anti-alias
	{
		mpAntiAliasPipelineState = createPostProcess("PostProcess/AntiAlias");
		VuShaderProgram *pSP = mpAntiAliasPipelineState->mpShaderProgram;

		mhSpConstAntiAliasTexelSize = pSP->getConstantByName("gTexelSize");
	}

	return true;
}

//*****************************************************************************
void VuPostProcess::release()
{
	mpCopyPipelineState->removeRef();
	mpGaussBlurPipelineState->removeRef();
	mpRadialBlurPipelineState->removeRef();
	mpTintContrastPipelineState->removeRef();
	mpTintContrastGammaPipelineState->removeRef();
	mpAntiAliasPipelineState->removeRef();
}

//*****************************************************************************
void VuPostProcess::copy(VuTexture *pInputTexture)
{
	VuPipelineState *pPS = mpCopyPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	if ( mhSpConstCopyTexelSize )
	{
		VuShaderProgram *pSP = pPS->mpShaderProgram;
		pSP->setConstantVector2(mhSpConstCopyTexelSize, VuVector2(1.0f/pInputTexture->getWidth(), 1.0f/pInputTexture->getHeight()));
	}

	VuGfx::IF()->setTexture(0, pInputTexture);

	drawFullScreenQuad();
}

//*****************************************************************************
void VuPostProcess::radialBlur(VuTexture *pInputTexture, float fAmount)
{
	VuPipelineState *pPS = mpRadialBlurPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	VuShaderProgram *pSP = pPS->mpShaderProgram;

	if ( mhSpConstRadialBlurTexelSize )
		pSP->setConstantVector2(mhSpConstRadialBlurTexelSize, VuVector2(1.0f/pInputTexture->getWidth(), 1.0f/pInputTexture->getHeight()));

	pSP->setConstantFloat(mhSpConstRadialBlurAmount, fAmount);

	VuGfx::IF()->setTexture(0, pInputTexture);

	drawFullScreenQuad();
}

//*****************************************************************************
void VuPostProcess::colorCorrection(VuTexture *pInputTexture, const VuColor &contrast, const VuColor &tint, float gammaMin, float gammaMax, float gammaCurve)
{
	if ( gammaMin == 0.0f && gammaMax == 1.0f && gammaCurve == 1.0f )
	{
		VuPipelineState *pPS = mpTintContrastPipelineState;
		VuGfx::IF()->setPipelineState(pPS);

		VuShaderProgram *pSP = pPS->mpShaderProgram;

		if ( mhSpConstTintContrastTexelSize )
			pSP->setConstantVector2(mhSpConstTintContrastTexelSize, VuVector2(1.0f/pInputTexture->getWidth(), 1.0f/pInputTexture->getHeight()));

		pSP->setConstantColor3(mhSpConstTintContrastContrast, contrast);
		pSP->setConstantColor3(mhSpConstTintContrastTint, tint);
	}
	else
	{
		VuPipelineState *pPS = mpTintContrastGammaPipelineState;
		VuGfx::IF()->setPipelineState(pPS);

		VuShaderProgram *pSP = pPS->mpShaderProgram;

		if (mhSpConstTintContrastGammaTexelSize)
			pSP->setConstantVector2(mhSpConstTintContrastGammaTexelSize, VuVector2(1.0f/pInputTexture->getWidth(), 1.0f/pInputTexture->getHeight()));

		float gammaInvScale = 1.0f/(gammaMax - gammaMin);

		pSP->setConstantColor3(mhSpConstTintContrastGammaContrast, contrast);
		pSP->setConstantColor3(mhSpConstTintContrastGammaTint, tint);
		pSP->setConstantVector3(mhSpConstTintContrastGammaMin, VuVector3(gammaMin, gammaMin, gammaMin));
		pSP->setConstantVector3(mhSpConstTintContrastGammaInvScale, VuVector3(gammaInvScale, gammaInvScale, gammaInvScale));
		pSP->setConstantVector3(mhSpConstTintContrastGammaCurve, VuVector3(gammaCurve, gammaCurve, gammaCurve));
	}

	VuGfx::IF()->setTexture(0, pInputTexture);

	drawFullScreenQuad();
}

//*****************************************************************************
void VuPostProcess::antiAlias(VuTexture *pInputTexture)
{
	VuPipelineState *pPS = mpAntiAliasPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	VuShaderProgram *pSP = pPS->mpShaderProgram;

	if ( mhSpConstAntiAliasTexelSize )
		pSP->setConstantVector2(mhSpConstAntiAliasTexelSize, VuVector2(1.0f/pInputTexture->getWidth(), 1.0f/pInputTexture->getHeight()));

	VuGfx::IF()->setTexture(0, pInputTexture);

	drawFullScreenQuad();
}

//*****************************************************************************
void VuPostProcess::gaussBlur(VuTexture *pInputTexture, float horzAmount, float vertAmount)
{
	VuPipelineState *pPS = mpGaussBlurPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	VuShaderProgram *pSP = pPS->mpShaderProgram;

	pSP->setConstantVector2(mhSpConstGaussBlurTexelOffset, VuVector2(horzAmount/pInputTexture->getWidth(), vertAmount/pInputTexture->getHeight()));

	VuGfx::IF()->setTexture(0, pInputTexture);

	drawFullScreenQuad();
}

//*****************************************************************************
void VuPostProcess::drawFullScreenQuad()
{
	struct Vertex
	{
		float mXyz[2];
		float mUv[2];
	};
	Vertex verts[4];
	verts[0].mXyz[0] = -1; verts[0].mXyz[1] = -1; verts[0].mUv[0] = 0; verts[0].mUv[1] = 1;
	verts[1].mXyz[0] =  1; verts[1].mXyz[1] = -1; verts[1].mUv[0] = 1; verts[1].mUv[1] = 1;
	verts[2].mXyz[0] = -1; verts[2].mXyz[1] =  1; verts[2].mUv[0] = 0; verts[2].mUv[1] = 0;
	verts[3].mXyz[0] =  1; verts[3].mXyz[1] =  1; verts[3].mUv[0] = 1; verts[3].mUv[1] = 0;

	VuGfx::IF()->setCullMode(VUGFX_CULL_NONE);
	VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getPostProcessDepthStencilState());

	VuGfx::IF()->beginFullScreenEffect();
	VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
	VuGfx::IF()->endFullScreenEffect();

	// restore default render state
	VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getDefaultDepthStencilState());
	VuGfx::IF()->setCullMode(VUGFX_CULL_CW);
}
