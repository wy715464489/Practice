//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Lens Water Manager
// 
//*****************************************************************************

#include "VuLensWaterManager.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/PostProcess/VuPostProcess.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Math/VuRand.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Dev/VuDevMenu.h"


// constants
#define MAX_DROPLET_COUNT 256
#define DROPLET_TEXTURE_WIDTH 32
#define DROPLET_TEXTURE_HEIGHT 32

// static variables
static bool sShowTexture = false;
static float sFadeSpeed = 1.0f;
static float sRadialSpreadMultiplier = 1.0f;
static float sSizeMin = 0.0250f;
static float sSizeMax = 0.0500f;
static float sDirChangeTimerMin = 0.25f;
static float sDirChangeTimerMax = 0.5f;
static float sDirChangeLatVelRange = 0.5f;
static float sAccelY = -0.5f;
static float sDropletLifetimeMin = 1.0f;
static float sDropletLifetimeMax = 3.0f;


struct VuDroplet
{
	VuVector2	mCurScreenPos;
	VuVector2	mLastScreenPos;
	VuVector2	mVelocity;
	float		mSize;
	float		mLifetime;
	float		mDirChangeTimer;
};

struct VuDropletVert
{
	VuVector2	mPos;
	VuVector2	mDropletUV;
};
VU_COMPILE_TIME_ASSERT(VuScratchPad::SIZE/(6*sizeof(VuDropletVert)) >= MAX_DROPLET_COUNT);


// implementation
class VuLensWaterManagerImpl : public VuLensWaterManager
{
protected:

	// called by engine
	friend class VuEngine;
	virtual bool	init();
	virtual void	release();

public:
	VuLensWaterManagerImpl();
	~VuLensWaterManagerImpl();

	virtual bool		isEnabled()              { return mEnabled; }
	virtual bool		isActive(int viewport)   { return mViewports[viewport].mActive; }

	virtual void		reset();

	virtual void		addDroplets(int viewport, float count)      { mViewports[viewport].mEmitterAccum += count; }
	virtual void		setRadialSpread(int viewport, float amount) { mViewports[viewport].mRadialAcceleration = amount*sRadialSpreadMultiplier; }

	virtual void		setViewportCount(int count);
	virtual void		updateTextureSize(int viewport, int width, int height) { mViewports[viewport].updateTextureSize(width, height, mEnabled); }
	virtual void		submit(int viewport, VuTexture *pSourceTexture, VuRenderTarget *pRenderTarget);

	virtual void		registerEmitter(VuEmitterIF *pEmitter)   { mEmitters.push_back(pEmitter); }
	virtual void		unregisterEmitter(VuEmitterIF *pEmitter) { mEmitters.removeSwap(pEmitter); }

private:
	typedef VuArray<VUUINT16> IndexBuffer;
	typedef VuArray<VuEmitterIF *> Emitters;

	void				tick(float fdt);
	void				draw(int viewport, VuTexture *pSourceTexture, VuRenderTarget *pRenderTarget, float fadeAmount, int dropletCount, VuDroplet *pDroplets);
	void				drawDroplets(int viewport, int dropletCount, VuDroplet *pDroplets);
	void				destroyResources();

	void				configLensWater(bool value) { mEnabled = value; }

	bool				mEnabled;

	VuPipelineState			*mpFadePipelineState;
	int						miSampFadePrevTexture;
	VUHANDLE				mhSpConstFadePrevTexelSize;
	VUHANDLE				mhSpConstFadeAmount;

	VuPipelineState			*mpDropletPipelineState;
	int						miSampDropletPrevTexture;
	int						miSampDropletDropletTexture;
	VUHANDLE				mhSpConstDropletScaleX;
	VUHANDLE				mhSpConstDropletPrevTexelSize;
	VuTexture				*mpDropletTexture;

	VuPipelineState			*mpEffectPipelineState;
	int						miSampEffectWaterTexture;
	int						miSampEffectColorTexture;
	VUHANDLE				mhSpConstEffectWaterTexelSize;
	VUHANDLE				mhSpConstEffectColorTexelSize;
	VUHANDLE				mhSpConstEffectHeightmapOffset;

	struct Viewport
	{
		Viewport();

		void				reset();
		void				updateTextureSize(int width, int height, bool enabled);
		void				tick(float fdt, const Emitters &emitters, const VuVector3 &cameraPos);
		void				destroyResources();

		int					mWidth;
		int					mHeight;
		float				mAspectRatio;
		VuRenderTarget		*mpSrcRenderTarget;
		VuRenderTarget		*mpDstRenderTarget;
		bool				mClear;

		float				mFadeAmount;
		float				mRadialAcceleration;
		float				mInactiveTimer;
		bool				mActive;

		VuDroplet			mDroplets[MAX_DROPLET_COUNT];
		int					mDropletCount;

		float				mEmitterAccum;
	};
	Viewport			mViewports[VuViewportManager::MAX_VIEWPORTS];
	int					mViewportCount;

	IndexBuffer			mIndexBuffer;

	Emitters			mEmitters;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuLensWaterManager, VuLensWaterManagerImpl);


//*****************************************************************************
VuLensWaterManagerImpl::VuLensWaterManagerImpl():
	mEnabled(true),
	mViewportCount(0),
	mIndexBuffer(0)
{
	// set up dev menu/stats
	VuDevMenu::IF()->addBool("LensWater/ShowTexture", sShowTexture);
	VuDevMenu::IF()->addFloat("LensWater/FadeSpeed", sFadeSpeed, 0.001f, 0.001f);
	VuDevMenu::IF()->addFloat("LensWater/RadialSpreadMultiplier", sRadialSpreadMultiplier, 0.01f, 0.0f);
	VuDevMenu::IF()->addFloat("LensWater/SizeMin", sSizeMin, 0.001f, 0.0f);
	VuDevMenu::IF()->addFloat("LensWater/SizeMax", sSizeMax, 0.001f, 0.0f);
	VuDevMenu::IF()->addFloat("LensWater/DirChangeTimerMin", sDirChangeTimerMin, 0.01f, 0.0f);
	VuDevMenu::IF()->addFloat("LensWater/DirChangeTimerMax", sDirChangeTimerMax, 0.01f, 0.0f);
	VuDevMenu::IF()->addFloat("LensWater/DirChangeLatVelRange", sDirChangeLatVelRange, 0.01f, -10.0f, 10.0f);
	VuDevMenu::IF()->addFloat("LensWater/AccelY", sAccelY, 0.01f);
	VuDevMenu::IF()->addFloat("LensWater/DropletLifetimeMin", sDropletLifetimeMin, 0.1f, 0.0f);
	VuDevMenu::IF()->addFloat("LensWater/DropletLifetimeMax", sDropletLifetimeMax, 0.1f, 0.0f);
}

//*****************************************************************************
VuLensWaterManagerImpl::~VuLensWaterManagerImpl()
{
	VUASSERT(mEmitters.size() == 0, "Lens water emitter leak!");

	destroyResources();
}

//*****************************************************************************
bool VuLensWaterManagerImpl::init()
{
	VuTickManager::IF()->registerHandler(this, &VuLensWaterManagerImpl::tick, "Build");

	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 0, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 8, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(16));

	// fader shader
	{
		VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("LensWater/Fade");
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		mpFadePipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		VuShaderProgram *pSP = mpFadePipelineState->mpShaderProgram;

		miSampFadePrevTexture = pSP->getSamplerIndexByName("PrevTexture");
		mhSpConstFadePrevTexelSize = pSP->getConstantByName("gPrevTexelSize");
		mhSpConstFadeAmount = pSP->getConstantByName("gAmount");

		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	}

	// droplet shader
	{
		VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("LensWater/Droplet");
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		mpDropletPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		VuShaderProgram *pSP = mpDropletPipelineState->mpShaderProgram;

		miSampDropletPrevTexture = pSP->getSamplerIndexByName("PrevTexture");
		miSampDropletDropletTexture = pSP->getSamplerIndexByName("DropletTexture");
		mhSpConstDropletScaleX = pSP->getConstantByName("gScaleX");
		mhSpConstDropletPrevTexelSize = pSP->getConstantByName("gPrevTexelSize");

		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	}

	// effect shader
	{
		VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("LensWater/Effect");
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		mpEffectPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		VuShaderProgram *pSP = mpEffectPipelineState->mpShaderProgram;

		miSampEffectWaterTexture = pSP->getSamplerIndexByName("WaterTexture");
		miSampEffectColorTexture = pSP->getSamplerIndexByName("ColorTexture");
		mhSpConstEffectWaterTexelSize = pSP->getConstantByName("gWaterTexelSize");
		mhSpConstEffectColorTexelSize = pSP->getConstantByName("gColorTexelSize");
		mhSpConstEffectHeightmapOffset = pSP->getConstantByName("gHeightmapOffset");

		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	}

	// droplet texture
	{
		VuTextureState state;
		state.mAddressU = VUGFX_ADDRESS_CLAMP;
		state.mAddressV = VUGFX_ADDRESS_CLAMP;
		state.mMipFilter = VUGFX_TEXF_NONE;
		mpDropletTexture = VuGfx::IF()->createTexture(DROPLET_TEXTURE_WIDTH, DROPLET_TEXTURE_HEIGHT,  0, VUGFX_FORMAT_LIN_R8, state);

		VuArray<VUBYTE> data(0);
		data.resize(DROPLET_TEXTURE_WIDTH*DROPLET_TEXTURE_HEIGHT);

		for ( int y = 0; y < DROPLET_TEXTURE_HEIGHT; y++ )
		{
			float fy = y*(2.0f/DROPLET_TEXTURE_HEIGHT) - 1.0f;
			for ( int x = 0; x < DROPLET_TEXTURE_WIDTH; x++ )
			{
				float fx = x*(2.0f/DROPLET_TEXTURE_WIDTH) - 1.0f;
				float dist = VuSqrt(fx*fx + fy*fy);
				dist = (dist - 0.5f)/0.5f;
				dist = VuClamp(dist, 0.0f, 1.0f);
				float value = VuSqrt(1.0f - dist);
				data[y*DROPLET_TEXTURE_WIDTH + x] = (VUUINT8)VuRound(255.0f*VuClamp(value, 0.0f, 1.0f));
			}
		}

		mpDropletTexture->setData(0, &data[0], data.size());
	}

	// index buffer
	{
		mIndexBuffer.resize(MAX_DROPLET_COUNT*12);
		VUUINT16 *pIndexData = &mIndexBuffer.begin();
		for ( VUUINT16 i = 0; i < MAX_DROPLET_COUNT; i++ )
		{
			VUUINT16 baseIndex = i*6;

			pIndexData[0] = baseIndex + 1;
			pIndexData[1] = baseIndex + 3;
			pIndexData[2] = baseIndex + 0;
			pIndexData[3] = baseIndex + 0;
			pIndexData[4] = baseIndex + 3;
			pIndexData[5] = baseIndex + 2;

			pIndexData[6] = baseIndex + 3;
			pIndexData[7] = baseIndex + 5;
			pIndexData[8] = baseIndex + 2;
			pIndexData[9] = baseIndex + 2;
			pIndexData[10] = baseIndex + 5;
			pIndexData[11] = baseIndex + 4;

			pIndexData += 12;
		}
	}

	// configuration
	mEnabled = VuConfigManager::IF()->getBool("Effects/LensWater")->mValue;
	VuConfigManager::IF()->registerBoolHandler("Effects/LensWater", this, &VuLensWaterManagerImpl::configLensWater);

	return true;
}

//*****************************************************************************
void VuLensWaterManagerImpl::release()
{
	VuConfigManager::IF()->unregisterBoolHandler("Effects/LensWater", this);

	VuTickManager::IF()->unregisterHandlers(this);

	mpFadePipelineState->removeRef();
	mpDropletPipelineState->removeRef();
	mpEffectPipelineState->removeRef();

	mpDropletTexture->removeRef();
}

//*****************************************************************************
void VuLensWaterManagerImpl::reset()
{
	for ( int i = 0; i < VuViewportManager::MAX_VIEWPORTS; i++ )
		mViewports[i].reset();
}

//*****************************************************************************
void VuLensWaterManagerImpl::setViewportCount(int count)
{
	if ( count != mViewportCount )
		destroyResources();

	mViewportCount = count;
}

//*****************************************************************************
void VuLensWaterManagerImpl::submit(int viewport, VuTexture *pSourceTexture, VuRenderTarget *pRenderTarget)
{
	if ( !mEnabled )
		return;

	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);
			pData->mpImpl->draw(pData->mViewport, pData->mpSourceTexture, pData->mpRenderTarget, pData->mFadeAmount, pData->mDropletCount, (VuDroplet *)(pData + 1));
		}

		VuLensWaterManagerImpl	*mpImpl;
		int						mViewport;
		VuTexture				*mpSourceTexture;
		VuRenderTarget			*mpRenderTarget;
		float					mFadeAmount;
		int						mDropletCount;
	};

	Viewport &vp = mViewports[viewport];

	int dropletsSize = vp.mDropletCount*sizeof(vp.mDroplets[0]);
	int size = sizeof(CommandData) + dropletsSize;
	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(size));

	pData->mpImpl = this;
	pData->mViewport = viewport;
	pData->mpSourceTexture = pSourceTexture;
	pData->mpRenderTarget = pRenderTarget;
	pData->mFadeAmount = vp.mFadeAmount;
	pData->mDropletCount = vp.mDropletCount;

	VU_MEMCPY(pData + 1, dropletsSize, vp.mDroplets, vp.mDropletCount*sizeof(vp.mDroplets[0]));

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, 0, &CommandData::callback);
}

//*****************************************************************************
void VuLensWaterManagerImpl::tick(float fdt)
{
	if ( !mEnabled )
	{
		reset();
		return;
	}

	for ( int i = 0; i < mViewportCount; i++ )
	{
		const VuVector3 &cameraPos = VuViewportManager::IF()->getViewport(i).mCamera.getEyePosition();
		mViewports[i].tick(fdt, mEmitters, cameraPos);
	}
}

//*****************************************************************************
void VuLensWaterManagerImpl::draw(int viewport, VuTexture *pSourceTexture, VuRenderTarget *pRenderTarget, float fadeAmount, int dropletCount, VuDroplet *pDroplets)
{
	VU_PROFILE_GFX("LensWater");

	Viewport &vp = mViewports[viewport];

	// initialize source RT if dirty
	if ( vp.mClear )
	{
		VuSetRenderTargetParams params(vp.mpSrcRenderTarget);
		params.mColorLoadAction = VuSetRenderTargetParams::LoadActionClear;
		params.mDepthLoadAction = VuSetRenderTargetParams::LoadActionClear;
		VuGfx::IF()->setRenderTarget(params);

		vp.mClear = false;
	}

	// start using destination RT
	{
		VuSetRenderTargetParams params(vp.mpDstRenderTarget);
		params.mColorLoadAction = VuSetRenderTargetParams::LoadActionLoad;
		VuGfx::IF()->setRenderTarget(params);
	}

	// fade out droplets
	{
		VuPipelineState *pPS = mpFadePipelineState;
		VuGfx::IF()->setPipelineState(pPS);

		VuShaderProgram *pSP = pPS->mpShaderProgram;

		VuGfx::IF()->setTexture(miSampFadePrevTexture, vp.mpSrcRenderTarget->getColorTexture());
		if ( mhSpConstFadePrevTexelSize )
			pSP->setConstantVector2(mhSpConstFadePrevTexelSize, VuVector2(1.0f/vp.mWidth, 1.0f/vp.mHeight));
		pSP->setConstantFloat(mhSpConstFadeAmount, VuTruncate(fadeAmount) / 255.0f);

		VuGfxUtil::IF()->postProcess()->drawFullScreenQuad();
	}

	// draw droplets
	if ( dropletCount )
	{
		drawDroplets(viewport, dropletCount, pDroplets);
	}

	// do effect
	{
		VuTexture *pWaterTexture = vp.mpDstRenderTarget->getColorTexture();

		VuGfx::IF()->setRenderTarget(VuSetRenderTargetParams(pRenderTarget));

		if ( sShowTexture )
		{
			VuGfxUtil::IF()->postProcess()->copy(pWaterTexture);
		}
		else
		{
			VuPipelineState *pPS = mpEffectPipelineState;
			VuGfx::IF()->setPipelineState(pPS);

			VuShaderProgram *pSP = pPS->mpShaderProgram;

			VuGfx::IF()->setTexture(miSampEffectWaterTexture, pWaterTexture);
			VuGfx::IF()->setTexture(miSampEffectColorTexture, pSourceTexture);

			if ( mhSpConstEffectWaterTexelSize )
				pSP->setConstantVector2(mhSpConstEffectWaterTexelSize, VuVector2(1.0f/vp.mWidth, 1.0f/vp.mHeight));

			if ( mhSpConstEffectColorTexelSize )
				pSP->setConstantVector2(mhSpConstEffectColorTexelSize, VuVector2(1.0f/pSourceTexture->getWidth(), 1.0f/pSourceTexture->getHeight()));

			VuVector2 vHeightmapOffset = VuVector2(1.0f/vp.mWidth, 1.0f/vp.mHeight);
			pSP->setConstantVector2(mhSpConstEffectHeightmapOffset, vHeightmapOffset);

			VuGfxUtil::IF()->postProcess()->drawFullScreenQuad();
		}
	}

	VuSwap(vp.mpSrcRenderTarget, vp.mpDstRenderTarget);
}

//*****************************************************************************
void VuLensWaterManagerImpl::drawDroplets(int viewport, int dropletCount, VuDroplet *pDroplets)
{
	Viewport &vp = mViewports[viewport];

	VuGfx::IF()->setCullMode(VUGFX_CULL_NONE);

	VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getPostProcessDepthStencilState());

	VuPipelineState *pPS = mpDropletPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	VuShaderProgram *pSP = pPS->mpShaderProgram;

	VuGfx::IF()->setTexture(miSampDropletPrevTexture, vp.mpSrcRenderTarget->getColorTexture());
	VuGfx::IF()->setTexture(miSampDropletDropletTexture, mpDropletTexture);
	pSP->setConstantFloat(mhSpConstDropletScaleX, 1.0f/vp.mAspectRatio);
	if ( mhSpConstDropletPrevTexelSize )
		pSP->setConstantVector2(mhSpConstDropletPrevTexelSize, VuVector2(1.0f/vp.mWidth, 1.0f/vp.mHeight));

	// batch verts using scratch pad
	void *pScratchPad = VuScratchPad::get(VuScratchPad::GRAPHICS);
	VuDropletVert *pVert = (VuDropletVert *)pScratchPad;

	VuDroplet *pDroplet = pDroplets;
	for ( int i = 0; i < dropletCount; i++ )
	{
		float halfSize = 0.5f*pDroplet->mSize;
		VuVector2 dirY = (pDroplet->mCurScreenPos - pDroplet->mLastScreenPos).safeNormal();
		VuVector2 dirX = VuVector2(dirY.mY, -dirY.mX); 

		pVert[0].mPos.mX = -halfSize;
		pVert[0].mPos.mY = 0.0f;
		pVert[0].mDropletUV.mX = 0.0f;
		pVert[0].mDropletUV.mY = 0.5f;

		pVert[1].mPos.mX = halfSize;
		pVert[1].mPos.mY = 0.0f;
		pVert[1].mDropletUV.mX = 1.0f;
		pVert[1].mDropletUV.mY = 0.5f;

		pVert[2].mPos.mX = -halfSize;
		pVert[2].mPos.mY = 0.0f;
		pVert[2].mDropletUV.mX = 0.0f;
		pVert[2].mDropletUV.mY = 0.5f;

		pVert[3].mPos.mX = halfSize;
		pVert[3].mPos.mY = 0.0f;
		pVert[3].mDropletUV.mX = 1.0f;
		pVert[3].mDropletUV.mY = 0.5f;

		pVert[4].mPos.mX = -halfSize;
		pVert[4].mPos.mY = halfSize;
		pVert[4].mDropletUV.mX = 0.0f;
		pVert[4].mDropletUV.mY = 1.0f;

		pVert[5].mPos.mX = halfSize;
		pVert[5].mPos.mY = halfSize;
		pVert[5].mDropletUV.mX = 1.0f;
		pVert[5].mDropletUV.mY = 1.0f;

		pVert[0].mPos = pDroplet->mLastScreenPos + dirX*pVert[0].mPos.mX + dirY*pVert[0].mPos.mY;
		pVert[1].mPos = pDroplet->mLastScreenPos + dirX*pVert[1].mPos.mX + dirY*pVert[1].mPos.mY;

		pVert[2].mPos = pDroplet->mCurScreenPos + dirX*pVert[2].mPos.mX + dirY*pVert[2].mPos.mY;
		pVert[3].mPos = pDroplet->mCurScreenPos + dirX*pVert[3].mPos.mX + dirY*pVert[3].mPos.mY;
		pVert[4].mPos = pDroplet->mCurScreenPos + dirX*pVert[4].mPos.mX + dirY*pVert[4].mPos.mY;
		pVert[5].mPos = pDroplet->mCurScreenPos + dirX*pVert[5].mPos.mX + dirY*pVert[5].mPos.mY;

		pDroplet++;
		pVert += 6;
	}

	// submit batch
	VuGfx::IF()->drawIndexedPrimitiveUP(
		VUGFX_PT_TRIANGLELIST,	// PrimitiveType
		0,						// MinVertexIndex
		dropletCount*6,			// NumVertices
		dropletCount*4,			// PrimitiveCount
		&mIndexBuffer[0],		// IndexData
		pScratchPad				// VertexStreamZeroData
	);

	// restore default render state
	VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getDefaultDepthStencilState());
	VuGfx::IF()->setCullMode(VUGFX_CULL_CW);
}

//*****************************************************************************
void VuLensWaterManagerImpl::destroyResources()
{
	for ( int i = 0; i < VuViewportManager::MAX_VIEWPORTS; i++ )
		mViewports[i].destroyResources();
}

//*****************************************************************************
VuLensWaterManagerImpl::Viewport::Viewport():
	mWidth(0),
	mHeight(0),
	mAspectRatio(1.0f),
	mpSrcRenderTarget(VUNULL),
	mpDstRenderTarget(VUNULL),
	mClear(false),
	mFadeAmount(0.0f),
	mRadialAcceleration(0.0f),
	mInactiveTimer(0.0f),
	mActive(false),
	mDropletCount(0),
	mEmitterAccum(0.0f)
{
}

//*****************************************************************************
void VuLensWaterManagerImpl::Viewport::reset()
{
	mDropletCount = 0;
	mRadialAcceleration = 0.0f;
	mClear = true;
	mEmitterAccum = 0.0f;
}

//*****************************************************************************
void VuLensWaterManagerImpl::Viewport::updateTextureSize(int width, int height, bool enabled)
{
	bool update = false;
	if ( mpSrcRenderTarget )
	{
		if ( !enabled )
			update = true;

		if ( mWidth != width || mHeight != height )
			update = true;
	}
	else
	{
		if ( enabled )
			update = true;
	}

	if ( update )
	{
		VuGfxSort::IF()->flush();

		destroyResources();

		if ( enabled )
		{
			mpSrcRenderTarget = VuGfx::IF()->createRenderTarget(width, height);
			mpDstRenderTarget = VuGfx::IF()->createRenderTarget(width, height);
			mClear = true;
		}

		mWidth = width;
		mHeight = height;
		mAspectRatio = (float)width/height;
	}
}

//*****************************************************************************
void VuLensWaterManagerImpl::Viewport::tick(float fdt, const Emitters &emitters, const VuVector3 &cameraPos)
{
	// update emitters
	for ( int i = 0; i < emitters.size(); i++ )
		mEmitterAccum += emitters[i]->lensWaterRate(cameraPos)*fdt;

	// add droplets
	if ( int dropletCount = VuTruncate(mEmitterAccum) )
	{
		mEmitterAccum -= dropletCount;

		dropletCount = VuMin(dropletCount, MAX_DROPLET_COUNT - mDropletCount);
		for ( int i = 0; i < dropletCount; i++ )
		{
			VuDroplet &droplet = mDroplets[mDropletCount++];

			droplet.mCurScreenPos.mX = VuRand::global().range(-mAspectRatio, mAspectRatio);
			droplet.mCurScreenPos.mY = VuRand::global().range(-1.0f, 1.0f);
			droplet.mLastScreenPos   = droplet.mCurScreenPos;
			droplet.mVelocity.mX     = 0.0f;
			droplet.mVelocity.mY     = 0.0f;
			droplet.mSize	         = VuRand::global().range(sSizeMin, sSizeMax);
			droplet.mLifetime        = VuRand::global().range(sDropletLifetimeMin, sDropletLifetimeMax);
			droplet.mDirChangeTimer  = VuRand::global().range(sDirChangeTimerMin, sDirChangeTimerMax);
		}
	}

	mFadeAmount -= VuTruncate(mFadeAmount);
	mFadeAmount += 255.0f*sFadeSpeed*fdt;

	if ( mDropletCount )
	{
		mActive = true;
		mInactiveTimer = 0.0f;
	}
	else
	{
		if ( mInactiveTimer > 1.0f/sFadeSpeed )
			mActive = false;
		mInactiveTimer += fdt;
	}

	for ( int i = 0; i < mDropletCount; i++ )
	{
		VuDroplet &droplet = mDroplets[i];

		// update age
		droplet.mLifetime -= fdt;

		// remove?
		if ( droplet.mLifetime <= 0.0f || VuAbs(droplet.mCurScreenPos.mX) > mAspectRatio || VuAbs(droplet.mCurScreenPos.mY) > 1.0f )
		{
			// swap with back
			mDroplets[i] = mDroplets[mDropletCount - 1];
			mDropletCount--;
			i--;
		}
		else
		{
			droplet.mDirChangeTimer -= fdt;
			if ( droplet.mDirChangeTimer < 0 )
			{
				VuVector2 latVel = VuVector2(droplet.mVelocity.mY, -droplet.mVelocity.mX);

				droplet.mDirChangeTimer = VuRand::global().range(sDirChangeTimerMin, sDirChangeTimerMax);
				droplet.mVelocity += latVel*VuRand::global().range(-sDirChangeLatVelRange, sDirChangeLatVelRange);
			}

			droplet.mLastScreenPos = droplet.mCurScreenPos;

			droplet.mCurScreenPos += droplet.mVelocity*fdt;

			VuVector2 dir = droplet.mCurScreenPos.safeNormal();

			droplet.mVelocity += dir*mRadialAcceleration*fdt;
			droplet.mVelocity.mY += sAccelY*fdt;
		}
	}
}

//*****************************************************************************
void VuLensWaterManagerImpl::Viewport::destroyResources()
{
	if ( mpSrcRenderTarget )
	{
		mpSrcRenderTarget->removeRef();
		mpSrcRenderTarget = VUNULL;
	}

	if ( mpDstRenderTarget )
	{
		mpDstRenderTarget->removeRef();
		mpDstRenderTarget = VUNULL;
	}
}

