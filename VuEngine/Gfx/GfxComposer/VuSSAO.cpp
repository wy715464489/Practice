//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SSAO implementation class
// 
//*****************************************************************************

#include "VuSSAO.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuDepthRenderTarget.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/PostProcess/VuPostProcess.h"
#include "VuEngine/Dev/VuDevMenu.h"


//*****************************************************************************
VuSSAO::VuSSAO():
	mEnabled(false),
	mWidth(0),
	mHeight(0),
	mpDepthRenderTarget(VUNULL),
	mpRenderTarget(VUNULL)
{
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("PostProcess/SSAO");

	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 0, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 8, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(16));
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

	VuPipelineStateParams psParams;
	mpEffectPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

	VuShaderProgram *pSP = mpEffectPipelineState->mpShaderProgram;

	mhSpConstTexelSize = pSP->getConstantByName("gTexelSize");
	mhSpConstScreenSize = pSP->getConstantByName("gScreenSize");
	mhSpConstNoiseSize = pSP->getConstantByName("gNoiseSize");
	miSampNoiseTexture = pSP->getSamplerIndexByName("NoiseTexture");
	miSampDepthTexture = pSP->getSamplerIndexByName("DepthTexture");

	mpNoiseTexture = VuAssetFactory::IF()->createAsset<VuTextureAsset>("Pfx/Noise");

	pVD->removeRef();
	VuAssetFactory::IF()->releaseAsset(pShaderAsset);
}

//*****************************************************************************
VuSSAO::~VuSSAO()
{
	destroyResources();
	VuAssetFactory::IF()->releaseAsset(mpNoiseTexture);
	mpEffectPipelineState->removeRef();
}

//*****************************************************************************
void VuSSAO::configure(bool enabled, int width, int height)
{
	// update?
	if ( enabled != mEnabled || width != mWidth || height != mHeight )
	{
		VuGfxSort::IF()->flush();

		destroyResources();

		mEnabled = enabled;
		mWidth = width;
		mHeight = height;

		if ( mEnabled )
		{
			mpDepthRenderTarget = VuGfx::IF()->createDepthRenderTarget(mWidth, mHeight);
			mpRenderTarget = VuGfx::IF()->createRenderTarget(mWidth, mHeight);
		}
	}
}

//*****************************************************************************
void VuSSAO::submitCommands()
{
	if ( !mEnabled )
		return;

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SSAO);

	// depth commands
	submitDepthCommands();

	// ssao full-screen effect
	submitEffectCommands();
}

//*****************************************************************************
VuTexture *VuSSAO::getTexture()
{
	if ( !mEnabled )
		return VuGfxUtil::IF()->whiteTexture();

	return mpRenderTarget->getColorTexture();
}

//*****************************************************************************
VuTexture *VuSSAO::getDepthTexture()
{
	if ( !mEnabled )
		return VuGfxUtil::IF()->whiteTexture();

	return mpDepthRenderTarget->getTexture();
}

//*****************************************************************************
void VuSSAO::resolveConstants(VuShaderProgram *pSP, VuMatExt *pMatExt)
{
	// PSSM
	pMatExt->mhConstConstScreenSize = pSP->getConstantByName("gScreenSize");
	pMatExt->miSampSSAOTexture = pSP->getSamplerIndexByName("SSAOTexture");
}

//*****************************************************************************
void VuSSAO::setConstants(VuShaderProgram *pSP, const VuMatExt *pMatExt)
{
	if ( pMatExt->mhConstConstScreenSize )
		pSP->setConstantVector2(pMatExt->mhConstConstScreenSize, VuVector2(mWidth, mHeight));

	if ( pMatExt->miSampSSAOTexture >= 0 )
	{
		if ( mEnabled )
			VuGfx::IF()->setTexture(pMatExt->miSampSSAOTexture, mpRenderTarget->getColorTexture());
		else
			VuGfx::IF()->setTexture(pMatExt->miSampSSAOTexture, VuGfxUtil::IF()->whiteTexture());
	}
}

//*****************************************************************************
void VuSSAO::destroyResources()
{
	if ( mpDepthRenderTarget )
	{
		mpDepthRenderTarget->removeRef();
		mpDepthRenderTarget = VUNULL;
	}

	if ( mpRenderTarget )
	{
		mpRenderTarget->removeRef();
		mpRenderTarget = VUNULL;
	}
}

//*****************************************************************************
void VuSSAO::submitDepthCommands()
{
	struct PreCommandData
	{
		static void callback(void *data)
		{
			PreCommandData *pData = static_cast<PreCommandData *>(data);

			VuGfx::IF()->setDepthRenderTarget(pData->mpDepthRenderTarget);
			VuGfx::IF()->setCullMode(VUGFX_CULL_CW);
		}
		VuDepthRenderTarget	*mpDepthRenderTarget;
	};

	PreCommandData *pPreData = static_cast<PreCommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(PreCommandData)));
	pPreData->mpDepthRenderTarget = mpDepthRenderTarget;
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &PreCommandData::callback);
}

//*****************************************************************************
void VuSSAO::submitEffectCommands()
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpSSAO->mpRenderTarget));

			VuPipelineState *pPS = pData->mpSSAO->mpEffectPipelineState;
			VuGfx::IF()->setPipelineState(pPS);

			VuShaderProgram *pSP = pData->mpSSAO->mpEffectPipelineState->mpShaderProgram;

			if ( pData->mpSSAO->mhSpConstTexelSize )
				pSP->setConstantVector2(pData->mpSSAO->mhSpConstTexelSize, VuVector2(1.0f/pData->mpSSAO->mpDepthRenderTarget->getTexture()->getWidth(), 1.0f/pData->mpSSAO->mpDepthRenderTarget->getTexture()->getHeight()));

			if ( pData->mpSSAO->mhSpConstScreenSize )
				pSP->setConstantVector2(pData->mpSSAO->mhSpConstScreenSize, VuVector2(pData->mpSSAO->mWidth, pData->mpSSAO->mHeight));

			if ( pData->mpSSAO->mhSpConstNoiseSize )
				pSP->setConstantVector2(pData->mpSSAO->mhSpConstNoiseSize, VuVector2(pData->mpSSAO->mpNoiseTexture->getTexture()->getWidth(), pData->mpSSAO->mpNoiseTexture->getTexture()->getHeight()));

			if ( pData->mpSSAO->miSampNoiseTexture >= 0 )
				VuGfx::IF()->setTexture(pData->mpSSAO->miSampNoiseTexture, pData->mpSSAO->mpNoiseTexture->getTexture());

			if ( pData->mpSSAO->miSampDepthTexture >= 0 )
				VuGfx::IF()->setTexture(pData->mpSSAO->miSampDepthTexture, pData->mpSSAO->mpDepthRenderTarget->getTexture());

			VuGfxUtil::IF()->postProcess()->drawFullScreenQuad();
		}

		VuSSAO	*mpSSAO;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpSSAO = this;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_END, 1, &CommandData::callback);
}
