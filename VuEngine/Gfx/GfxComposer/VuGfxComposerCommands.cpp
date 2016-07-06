//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxComposerCommands implementation class
// 
//*****************************************************************************

#include "VuGfxComposerCommands.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/VuGfxSettings.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/PostProcess/VuPostProcess.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"
#include "VuEngine/HAL/Gfx/VuShadowRenderTarget.h"


//*****************************************************************************
void VuGfxComposerSceneCommands::submitClear(VuRenderTarget *pRenderTarget)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuSetRenderTargetParams params(pData->mpRenderTarget);
			params.mColorLoadAction = VuSetRenderTargetParams::LoadActionClear;
			params.mClearColor = VuGfxSort::IF()->getRenderGfxSettings().mClearColor;
			params.mDepthLoadAction = VuSetRenderTargetParams::LoadActionClear;
			VuGfx::IF()->setRenderTarget(params);
		}
		VuRenderTarget	*mpRenderTarget;
	};

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_DEPTH);

	CommandData *pPreData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pPreData->mpRenderTarget = pRenderTarget;
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &CommandData::callback);
}

//*****************************************************************************
void VuGfxComposerSceneCommands::submitReflectionClip(const VuVector4 &vPlane)
{
	struct SetClipPlaneData
	{
		static void callback(void *data)
		{
			SetClipPlaneData *pData = static_cast<SetClipPlaneData *>(data);

			VuGfx::IF()->setClipPlane(pData->mPlane);
		}
		VuVector4	mPlane;
	};

	struct DisableClipPlaneData
	{
		static void callback(void *data)
		{
			VuVector4 plane(0,0,0,0);
			VuGfx::IF()->setClipPlane(plane);
		}
	};

	SetClipPlaneData *pData = static_cast<SetClipPlaneData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(SetClipPlaneData)));
	pData->mPlane = vPlane;

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_DEPTH);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &SetClipPlaneData::callback);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_END, 0, &DisableClipPlaneData::callback);

	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &SetClipPlaneData::callback);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_END, 0, &DisableClipPlaneData::callback);
}

//*****************************************************************************
void VuGfxComposerSceneCommands::submitShadow(VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	struct PreCommandData
	{
		static void callback(void *data)
		{
			PreCommandData *pData = static_cast<PreCommandData *>(data);

			VuGfxUtil::IF()->setDefaultRenderState();
			VuGfx::IF()->setShadowRenderTarget(pData->mpShadowRenderTarget, pData->mLayer);
			VuGfx::IF()->setCullMode(VUGFX_CULL_CCW);
		}
		VuShadowRenderTarget	*mpShadowRenderTarget;
		int						mLayer;
	};
	struct PostCommandData
	{
		static void callback(void *data)
		{
			PostCommandData *pData = static_cast<PostCommandData *>(data);

			VuGfxUtil::IF()->setDefaultRenderState();
			pData->mpShadowRenderTarget->resolve(pData->mLayer);
			VuGfx::IF()->setCullMode(VUGFX_CULL_CW);
		}
		VuShadowRenderTarget	*mpShadowRenderTarget;
		int						mLayer;
	};

	PreCommandData *pPreData = static_cast<PreCommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(PreCommandData)));
	pPreData->mpShadowRenderTarget = pShadowRenderTarget;
	pPreData->mLayer = layer;
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &PreCommandData::callback);

	PostCommandData *pPostData = static_cast<PostCommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(PostCommandData)));
	pPostData->mpShadowRenderTarget = pShadowRenderTarget;
	pPostData->mLayer = layer;
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_END, 0, &PostCommandData::callback);
}

//*****************************************************************************
void VuGfxComposerSceneCommands::submitBeginEndScene(VUHANDLE context)
{
	struct BeginData
	{
		static void callback(void *data)
		{
			BeginData *pData = static_cast<BeginData *>(data);
			VuGfx::IF()->beginScene(pData->mContext);
			VuGfxUtil::IF()->setDefaultRenderState();
		}
		VUHANDLE mContext;
	};
	struct EndData
	{
		static void callback(void *data)
		{
			EndData *pData = static_cast<EndData *>(data);
			VuGfx::IF()->endScene(pData->mContext);
		}
		VUHANDLE mContext;
	};

	int fsl = VuGfxSort::IF()->getFullScreenLayer();
	int vp = VuGfxSort::IF()->getViewport();
	int rl = VuGfxSort::IF()->getReflectionLayer();
	int vpl = VuGfxSort::IF()->getViewportLayer();

	BeginData *pBeginData = static_cast<BeginData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(BeginData)));
	pBeginData->mContext = context;
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_BEGIN);
	VuGfxSort::IF()->setViewport(0);
	VuGfxSort::IF()->setReflectionLayer(0);
	VuGfxSort::IF()->setViewportLayer(0);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &BeginData::callback);

	EndData *pEndData = static_cast<EndData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(EndData)));
	pEndData->mContext = context;
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_END);
	VuGfxSort::IF()->setViewport(GFX_SORT_MAX_VIEWPORT_COUNT - 1);
	VuGfxSort::IF()->setReflectionLayer(1);
	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_END);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_END, 255, &EndData::callback);

	VuGfxSort::IF()->setFullScreenLayer(fsl);
	VuGfxSort::IF()->setViewport(vp);
	VuGfxSort::IF()->setReflectionLayer(rl);
	VuGfxSort::IF()->setViewportLayer(vpl);
}

//*****************************************************************************
void VuGfxComposerPostProcessCommands::copy(VuTexture *pTexture, VuRenderTarget *pRenderTarget, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget));

			// stretch-copy to framebuffer
			VuGfxUtil::IF()->postProcess()->copy(pData->mpTexture);
		}
		VuTexture		*mpTexture;
		VuRenderTarget	*mpRenderTarget;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpTexture = pTexture;
	pData->mpRenderTarget = pRenderTarget;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
}

//*****************************************************************************
void VuGfxComposerPostProcessCommands::radialBlur(VuTexture *pTexture, VuRenderTarget *pRenderTarget, float amount, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget));
			VuGfxUtil::IF()->postProcess()->radialBlur(pData->mpTexture, pData->mAmount);
		}
		VuTexture		*mpTexture;
		VuRenderTarget	*mpRenderTarget;
		float			mAmount;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpTexture = pTexture;
	pData->mpRenderTarget = pRenderTarget;
	pData->mAmount = amount;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
}

//*****************************************************************************
void VuGfxComposerPostProcessCommands::colorCorrection(VuTexture *pTexture, VuRenderTarget *pRenderTarget, const VuColor &contrast, const VuColor &tint, float gammaMin, float gammaMax, float gammaCurve, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget));
			VuGfxUtil::IF()->postProcess()->colorCorrection(pData->mpTexture, pData->mContrast, pData->mTint, pData->mGammaMin, pData->mGammaMax, pData->mGammaCurve);
		}
		VuTexture		*mpTexture;
		VuRenderTarget	*mpRenderTarget;
		VuColor			mContrast;
		VuColor			mTint;
		float			mGammaMin;
		float			mGammaMax;
		float			mGammaCurve;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpTexture = pTexture;
	pData->mpRenderTarget = pRenderTarget;
	pData->mContrast = contrast;
	pData->mTint = tint;
	pData->mGammaMin = gammaMin;
	pData->mGammaMax = gammaMax;
	pData->mGammaCurve = gammaCurve;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
}

//*****************************************************************************
void VuGfxComposerPostProcessCommands::antiAlias(VuTexture *pTexture, VuRenderTarget *pRenderTarget)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget));
			VuGfxUtil::IF()->postProcess()->antiAlias(pData->mpTexture);
		}
		VuTexture		*mpTexture;
		VuRenderTarget	*mpRenderTarget;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpTexture = pTexture;
	pData->mpRenderTarget = pRenderTarget;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &CommandData::callback);
}

//*****************************************************************************
void VuGfxComposerPostProcessCommands::blur(VuRenderTarget *pRenderTarget0, VuRenderTarget *pRenderTarget1, float amount, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			// horizontal pass
			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget1));
			VuGfxUtil::IF()->postProcess()->gaussBlur(pData->mpRenderTarget0->getColorTexture(), pData->mAmount, 0.0f);

			VuGfx::IF()->setTexture(0, VUNULL);

			// vertical pass
			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget0));
			VuGfxUtil::IF()->postProcess()->gaussBlur(pData->mpRenderTarget1->getColorTexture(), 0.0f, pData->mAmount);
		}
		VuRenderTarget	*mpRenderTarget0;
		VuRenderTarget	*mpRenderTarget1;
		float			mAmount;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpRenderTarget0 = pRenderTarget0;
	pData->mpRenderTarget1 = pRenderTarget1;
	pData->mAmount = amount;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
}

//*****************************************************************************
void VuGfxComposerPostProcessCommands::copyMulti(VuRenderTarget *pRenderTarget, const CopyMultiParams &params, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pData->mpRenderTarget));

			for ( int i = 0; i < pData->mParams.mCount; i++ )
			{
				VuGfx::IF()->setViewport(pData->mParams.mDstRects[i]);
				VuGfxUtil::IF()->postProcess()->copy(pData->mParams.mTextures[i]);
			}

			VuGfx::IF()->setViewport(VuRect(0,0,1,1));
		}
		VuRenderTarget	*mpRenderTarget;
		CopyMultiParams	mParams;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mpRenderTarget = pRenderTarget;
	pData->mParams = params;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
}
