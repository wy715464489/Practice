//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  HBAO implementation class
// 
//*****************************************************************************

#include "VuHBAO.h"
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
#include "VuEngine/HAL/Gfx/VuFxRenderTarget.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/PostProcess/VuPostProcess.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxSettings.h"
#include "VuEngine/Math/VuRand.h"


#define NOISE_TEXTURE_SIZE 32
#define NUM_DIRECTIONS 8

#define POSTFXBLUR_MAX_SAMPLES	10
#define KERNEL_RADIUS	(POSTFXBLUR_MAX_SAMPLES/2)
#define INV_LN2 1.44269504f


//*****************************************************************************
VuHBAO::VuHBAO():
	mEnabled(false),
	mWidth(0),
	mHeight(0),
	mpDepthRenderTarget(VUNULL),
	mpRenderTarget0(VUNULL),
	mpRenderTarget1(VUNULL)
{
	// HBAO effect
	{
		VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("PostProcess/HBAO");

		VuVertexDeclarationParams vdParams;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 0, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_POSITION, 0));
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 8, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
		vdParams.mStreams.push_back(VuVertexDeclarationStream(16));
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		mpHBAOPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		VuShaderProgram *pSP = mpHBAOPipelineState->mpShaderProgram;

		mhConstRadius = pSP->getConstantByName("gRadiusParams");
		mhConstBias = pSP->getConstantByName("gBiasParams");
		mhConstScreen = pSP->getConstantByName("gScreenParams");
		mhConstUvToView = pSP->getConstantByName("gUvToViewParams");
		mhConstFocal = pSP->getConstantByName("gFocalParams");
		mhConstNearFarPlanes = pSP->getConstantByName("gNearFarPlanes");
		miSampDepthTexture = pSP->getSamplerIndexByName("DepthTexture");
		miSampNoiseTexture = pSP->getSamplerIndexByName("NoiseTexture");

		mpNoiseTexture = createNoiseTexture();

		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	}

	// Blur effect
	{
		VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("PostProcess/HBAOBlur");

		VuVertexDeclarationParams vdParams;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 0, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_POSITION, 0));
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 8, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
		vdParams.mStreams.push_back(VuVertexDeclarationStream(16));
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		mpBlurPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		VuShaderProgram *pSP = mpBlurPipelineState->mpShaderProgram;

		mhConstBlurFactors = pSP->getConstantByName("gBlurFactors");

		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	}
}

//*****************************************************************************
VuHBAO::~VuHBAO()
{
	destroyResources();
	mpNoiseTexture->removeRef();
	mpHBAOPipelineState->removeRef();
	mpBlurPipelineState->removeRef();
}

//*****************************************************************************
void VuHBAO::configure(bool enabled, int width, int height)
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
			mpRenderTarget0 = VuGfx::IF()->createFxRenderTarget(mWidth, mHeight, VUGFX_FORMAT_R16G16F);
			mpRenderTarget1 = VuGfx::IF()->createFxRenderTarget(mWidth, mHeight, VUGFX_FORMAT_R16G16F);
		}
	}
}

//*****************************************************************************
void VuHBAO::submitCommands()
{
	if ( !mEnabled )
		return;

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_SSAO);

	// depth commands
	submitDepthCommands();

	// HBAO full-screen effect
	submitEffectCommands();
}

//*****************************************************************************
VuTexture *VuHBAO::getTexture()
{
	if ( !mEnabled )
		return VuGfxUtil::IF()->whiteTexture();

	return mpRenderTarget0->getTexture();
}

//*****************************************************************************
VuTexture *VuHBAO::getDepthTexture()
{
	if ( !mEnabled )
		return VuGfxUtil::IF()->whiteTexture();

	return mpDepthRenderTarget->getTexture();
}

//*****************************************************************************
VuTexture *VuHBAO::getNoiseTexture()
{
	if ( !mEnabled )
		return VuGfxUtil::IF()->whiteTexture();

	return mpNoiseTexture;
}

//*****************************************************************************
void VuHBAO::resolveConstants(VuShaderProgram *pSP, VuMatExt *pMatExt)
{
	// PSSM
	pMatExt->mhConstConstScreenSize = pSP->getConstantByName("gScreenSize");
	pMatExt->mhConstSSAOConsts = pSP->getConstantByName("gSSAOConsts");
	pMatExt->miSampSSAOTexture = pSP->getSamplerIndexByName("SSAOTexture");
}

//*****************************************************************************
void VuHBAO::setConstants(VuShaderProgram *pSP, const VuMatExt *pMatExt)
{
	if ( pMatExt->mhConstConstScreenSize )
	{
		int renderTargetWidth, renderTargetHeight;
		VuGfx::IF()->getCurRenderTargetSize(renderTargetWidth, renderTargetHeight);
		pSP->setConstantVector2(pMatExt->mhConstConstScreenSize, VuVector2(renderTargetWidth, renderTargetHeight));
	}

	if ( pMatExt->mhConstSSAOConsts )
	{
		const VuGfxSettings &settings = VuGfxSort::IF()->getRenderGfxSettings();
		pSP->setConstantVector2(pMatExt->mhConstSSAOConsts, VuVector2(settings.mHBAOLightFactor, settings.mHBAOAmbientFactor));
	}

	if ( pMatExt->miSampSSAOTexture >= 0 )
	{
		if ( mEnabled )
			VuGfx::IF()->setTexture(pMatExt->miSampSSAOTexture, mpRenderTarget0->getTexture());
		else
			VuGfx::IF()->setTexture(pMatExt->miSampSSAOTexture, VuGfxUtil::IF()->whiteTexture());
	}
}

//*****************************************************************************
VuTexture *VuHBAO::createNoiseTexture()
{
	// calculate data
	VUINT16 *pData = new VUINT16[NOISE_TEXTURE_SIZE*NOISE_TEXTURE_SIZE*8];
	for (int i = 0; i < NOISE_TEXTURE_SIZE; i ++)
	{
		for (int j = 0; j < NOISE_TEXTURE_SIZE; j++)
		{
			float r1 = VuRand::global().rand();
			float r2 = VuRand::global().rand();

			float angle = 2.0f * VU_PI * r1 / NUM_DIRECTIONS;

			float sinAngle, cosAngle;
			VuSinCos(angle, sinAngle, cosAngle);

			int offset = (i*NOISE_TEXTURE_SIZE + j)*4;

			pData[offset + 0] = (VUINT16)VuRound(32767*cosAngle);
			pData[offset + 1] = (VUINT16)VuRound(32767*sinAngle);
			pData[offset + 2] = (VUINT16)VuRound(32767*r2);
			pData[offset + 3] = 0;
		}
	}

	// create texture
	VuTextureState state;
	state.mMagFilter = VUGFX_TEXF_POINT;
	state.mMinFilter = VUGFX_TEXF_POINT;
	state.mMipFilter = VUGFX_TEXF_NONE;

	VuTexture *pTexture = VuGfx::IF()->createTexture(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 0, VUGFX_FORMAT_LIN_R16G16B16A16_SNORM, state);
	pTexture->setData(0, pData, NOISE_TEXTURE_SIZE*NOISE_TEXTURE_SIZE*8);

	// clean up
	delete[] pData;

	return pTexture;
}

//*****************************************************************************
void VuHBAO::destroyResources()
{
	VU_SAFE_RELEASE(mpDepthRenderTarget);
	VU_SAFE_RELEASE(mpRenderTarget0);
	VU_SAFE_RELEASE(mpRenderTarget1);
}

//*****************************************************************************
void VuHBAO::submitDepthCommands()
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
void VuHBAO::submitEffectCommands()
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			// HBAO effect
			{
				VuGfx::IF()->setFxRenderTarget(pData->mpHBAO->mpRenderTarget0);

				VuPipelineState *pPS = pData->mpHBAO->mpHBAOPipelineState;
				VuGfx::IF()->setPipelineState(pPS);

				VuShaderProgram *pSP = pPS->mpShaderProgram;

				const VuCamera &camera = VuGfxSort::IF()->getRenderCamera();

				VuHBAOConsts hbaoConsts;
				const VuGfxSettings &settings = VuGfxSort::IF()->getRenderGfxSettings();
				calcHBAOConsts(pData->mpHBAO->mWidth, pData->mpHBAO->mHeight, camera.getFovVert(), settings, hbaoConsts);

				if ( pData->mpHBAO->mhConstRadius )
					pSP->setConstantVector4(pData->mpHBAO->mhConstRadius, hbaoConsts.mRadiusParams);

				if ( pData->mpHBAO->mhConstBias )
					pSP->setConstantVector4(pData->mpHBAO->mhConstBias, hbaoConsts.mBiasParams);

				if ( pData->mpHBAO->mhConstScreen )
					pSP->setConstantVector4(pData->mpHBAO->mhConstScreen, hbaoConsts.mScreenParams);

				if ( pData->mpHBAO->mhConstUvToView )
					pSP->setConstantVector4(pData->mpHBAO->mhConstUvToView, hbaoConsts.mUvToViewParams);

				if ( pData->mpHBAO->mhConstFocal )
					pSP->setConstantVector4(pData->mpHBAO->mhConstFocal, hbaoConsts.mFocalParams);

				if ( pData->mpHBAO->mhConstNearFarPlanes )
					pSP->setConstantVector2(pData->mpHBAO->mhConstNearFarPlanes, VuVector2(camera.getNearPlane(), camera.getFarPlane()));

				if ( pData->mpHBAO->miSampDepthTexture >= 0 )
					VuGfx::IF()->setTexture(pData->mpHBAO->miSampDepthTexture, pData->mpHBAO->mpDepthRenderTarget->getTexture());

				if ( pData->mpHBAO->miSampNoiseTexture >= 0 )
					VuGfx::IF()->setTexture(pData->mpHBAO->miSampNoiseTexture, pData->mpHBAO->mpNoiseTexture);

				VuGfxUtil::IF()->postProcess()->drawFullScreenQuad();
			}

			// blur passes
			{
				float blurSigma = ((float)KERNEL_RADIUS + 1.0f) * 0.5f;
				float blurFalloff = INV_LN2 / (2.0f*blurSigma*blurSigma);
				VuVector4 blurFactorsH(1.0f/pData->mpHBAO->mWidth, 0.0, blurFalloff, 1.0);
				VuVector4 blurFactorsV(0.0, 1.0f/pData->mpHBAO->mHeight, blurFalloff, 1.0);

				VuPipelineState *pPS = pData->mpHBAO->mpBlurPipelineState;
				VuGfx::IF()->setPipelineState(pPS);

				VuShaderProgram *pSP = pPS->mpShaderProgram;

				for ( int i = 0; i < 2; i++ )
				{
					// Horizontal pass
					VuGfx::IF()->setFxRenderTarget(pData->mpHBAO->mpRenderTarget1);
					VuGfx::IF()->setTexture(0, pData->mpHBAO->mpRenderTarget0->getTexture());

					if ( pData->mpHBAO->mhConstBlurFactors )
						pSP->setConstantVector4(pData->mpHBAO->mhConstBlurFactors, blurFactorsH);

					VuGfxUtil::IF()->postProcess()->drawFullScreenQuad();
					VuGfx::IF()->setTexture(0, VUNULL);

					// Vertical pass
					VuGfx::IF()->setFxRenderTarget(pData->mpHBAO->mpRenderTarget0);
					VuGfx::IF()->setTexture(0, pData->mpHBAO->mpRenderTarget1->getTexture());

					if ( pData->mpHBAO->mhConstBlurFactors )
						pSP->setConstantVector4(pData->mpHBAO->mhConstBlurFactors, blurFactorsV);

					VuGfxUtil::IF()->postProcess()->drawFullScreenQuad();
					VuGfx::IF()->setTexture(0, VUNULL);
				}
			}
		}

		VuHBAO			*mpHBAO;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpHBAO = this;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_END, 1, &CommandData::callback);
}

//*****************************************************************************
void VuHBAO::calcHBAOConsts(int inputWidth, int inputHeight, float fovY, const VuGfxSettings &gfxSettings, VuHBAOConsts &consts)
{
	float rad = gfxSettings.mHBAORadius;
	float radSq = rad*rad;
	float negInvRadSq = -1.0f/radSq;

	float angleBias = VuDegreesToRadians(gfxSettings.mHBAOAngleBias);
	float tanAngleBias = VuTan(angleBias);

	float aoResX = (float)inputWidth;
	float aoResY = (float)inputHeight;

	float maxRadius = gfxSettings.mHBAOMaxRadius * VuMin(aoResX, aoResY);

	float invAoResX = 1.0f / aoResX;
	float invAoResY = 1.0f / aoResY;

	float focal1 = 1.0f / tanf(fovY * 0.5f) * (aoResY / aoResX);
	float focal2 = 1.0f / tanf(fovY * 0.5f);
	float invFocal1 = 1.0f / focal1;
	float invFocal2 = 1.0f / focal2;

	float uvToVA0 = 2.0f * invFocal1;
	float uvToVA1 = -2.0f * invFocal2;
	float uvToVB0 = -1.0f * invFocal1;
	float uvToVB1 = 1.0f * invFocal2;

	consts.mRadiusParams.set(rad, radSq, negInvRadSq, maxRadius);
	consts.mBiasParams.set(angleBias, tanAngleBias, gfxSettings.mHBAOStrength, 1.0);
	consts.mScreenParams.set(aoResX, aoResY, invAoResX, invAoResY);
	consts.mUvToViewParams.set(uvToVA0, uvToVA1, uvToVB0, uvToVB1);
	consts.mFocalParams.set(focal1, focal2, invFocal1, invFocal2);
}