//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx util
// 
//*****************************************************************************

#include "VuGfxUtil.h"
#include "Shaders/VuBasicShaders.h"
#include "Shaders/VuCollisionShader.h"
#include "Shaders/VuDepthShader.h"
#include "Shaders/VuShadowShader.h"
#include "Shaders/VuBlobShadowShader.h"
#include "Shaders/VuDropShadowShader.h"
#include "Font/VuFontDraw.h"
#include "PostProcess/VuPostProcess.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCubeTextureAsset.h"
#include "VuEngine/Assets/VuMaterialAsset.h"


#define BLACK_VERTEX_COLOR_COUNT 16384

static inline int VuVertexCountToPrimitiveCount(VuGfxPrimitiveType type, int vertexCount)
{
	switch(type)
	{
	case VUGFX_PT_POINTLIST:
		return vertexCount;

	case VUGFX_PT_LINELIST:
		return vertexCount / 2;

	case VUGFX_PT_LINESTRIP:
		return vertexCount - 1;

	case VUGFX_PT_TRIANGLELIST:
		return vertexCount / 3;

	case VUGFX_PT_TRIANGLESTRIP:
		return vertexCount - 2;

	default:
		VUASSERT(0, "Unknown primitive type");
		return 0;
	}
}

static VuBasicShaders::eFlavor sBasicShaderFlavorLookup[] =
{
	VuBasicShaders::FLV_OPAQUE,    // TRANS_BEGIN,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_OPAQUE,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_ALPHA_TEST,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_FOLIAGE,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_SKYBOX,
	VuBasicShaders::FLV_MODULATED, // TRANS_TIRE_TRACK,
	VuBasicShaders::FLV_MODULATED, // TRANS_BLOB_SHADOW,
	VuBasicShaders::FLV_MODULATED, // TRANS_MODULATE_BELOW_WATER,
	VuBasicShaders::FLV_ADDITIVE,  // TRANS_ADDITIVE_BELOW_WATER,
	VuBasicShaders::FLV_MODULATED, // TRANS_WATER_COLOR,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_DEPTH_PASS,
	VuBasicShaders::FLV_MODULATED, // TRANS_COLOR_PASS,
	VuBasicShaders::FLV_MODULATED, // TRANS_MODULATE_ABOVE_WATER,
	VuBasicShaders::FLV_ADDITIVE,  // TRANS_ADDITIVE_ABOVE_WATER,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_WATER_DEPTH,
	VuBasicShaders::FLV_MODULATED, // TRANS_MODULATE_CLIP_WATER,
	VuBasicShaders::FLV_ADDITIVE,  // TRANS_ADDITIVE_CLIP_WATER,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_UI_OPAQUE,
	VuBasicShaders::FLV_MODULATED, // TRANS_UI_MODULATE,
	VuBasicShaders::FLV_ADDITIVE,  // TRANS_UI_ADDITIVE,
	VuBasicShaders::FLV_OPAQUE,    // TRANS_END,
};
VU_COMPILE_TIME_ASSERT(sizeof(sBasicShaderFlavorLookup)/sizeof(sBasicShaderFlavorLookup[0]) == VuGfxSort::TRANS_END + 1);


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGfxUtil, VuGfxUtil);


//*****************************************************************************
VuGfxUtil::VuGfxUtil():
	mLowTextureLOD(false),
	mLowModelLOD(false),
	mUltraModelLOD(false),
	mShaderLOD(0),
	mQuadIndexBuffer(0)
{
	mMatrixStack.push(VuMatrix::identity());
	mTextScaleStack.push(1.0f);

	growQuadIndexBuffer(256);

	mpBasicShaders = new VuBasicShaders;
	mpCollisionShader = new VuCollisionShader;
	mpDepthShader = new VuDepthShader;
	mpShadowShader = new VuShadowShader;
	mpBlobShadowShader = new VuBlobShadowShader;
	mpDropShadowShader = new VuDropShadowShader;
	mpFontDraw = new VuFontDraw;
	mpPostProcess = new VuPostProcess;

	// configuration
	mLowTextureLOD = VuConfigManager::IF()->getBool("Gfx/LowTextureLOD")->mValue;
	mLowModelLOD = VuConfigManager::IF()->getBool("Gfx/LowModelLOD")->mValue;
	mUltraModelLOD = VuConfigManager::IF()->getBool("Gfx/UltraModelLOD")->mValue;
	mShaderLOD = VuConfigManager::IF()->getInt("Gfx/ShaderLOD")->mValue;

	VuConfigManager::IF()->registerBoolHandler("Gfx/LowTextureLOD", this, &VuGfxUtil::configLowTextureLOD);
	VuConfigManager::IF()->registerBoolHandler("Gfx/LowModelLOD", this, &VuGfxUtil::configLowModelLOD);
	VuConfigManager::IF()->registerBoolHandler("Gfx/UltraModelLOD", this, &VuGfxUtil::configUltraModelLOD);
	VuConfigManager::IF()->registerIntHandler("Gfx/ShaderLOD", this, &VuGfxUtil::configShaderLOD);
}

//*****************************************************************************
VuGfxUtil::~VuGfxUtil()
{
	VuConfigManager::IF()->unregisterBoolHandler("Gfx/LowTextureLOD", this);
	VuConfigManager::IF()->unregisterBoolHandler("Gfx/LowModelLOD", this);
	VuConfigManager::IF()->unregisterBoolHandler("Gfx/UltraModelLOD", this);
	VuConfigManager::IF()->unregisterIntHandler("Gfx/ShaderLOD", this);

	delete mpBasicShaders;
	delete mpCollisionShader;
	delete mpDepthShader;
	delete mpShadowShader;
	delete mpBlobShadowShader;
	delete mpDropShadowShader;
	delete mpFontDraw;
	delete mpPostProcess;
}

//*****************************************************************************
bool VuGfxUtil::init()
{
	if ( !mpBasicShaders->init() )
		return false;

	if ( !mpCollisionShader->init() )
		return false;

	if ( !mpDepthShader->init() )
		return false;

	if ( !mpShadowShader->init() )
		return false;

	if ( !mpBlobShadowShader->init() )
		return false;

	if ( !mpDropShadowShader->init() )
		return false;

	if ( !mpFontDraw->init() )
		return false;

	if ( !mpPostProcess->init() )
		return false;

	mpWhiteTexture = createWhiteTexture(32, 32);
	if ( !mpWhiteTexture )
		return false;

	mpBlackVertexColors = createBlackVertexColors();
	if ( !mpBlackVertexColors )
		return false;

	VuDepthStencilStateParams dssParams;
	mpDefaultDepthStencilState = VuGfx::IF()->createDepthStencilState(dssParams);

	dssParams.reset();
	dssParams.mDepthCompFunc = VUGFX_COMP_LESSEQUAL;
	mpLessEqualDepthStencilState = VuGfx::IF()->createDepthStencilState(dssParams);

	dssParams.reset();
	dssParams.mDepthCompFunc = VUGFX_COMP_ALWAYS;
	dssParams.mDepthWriteEnabled = false;
	mpPostProcessDepthStencilState = VuGfx::IF()->createDepthStencilState(dssParams);

	// clear color/depth depth-stencil states
	{
		dssParams.reset();
		dssParams.mDepthCompFunc = VUGFX_COMP_ALWAYS;
		dssParams.mDepthWriteEnabled = true;
		mpClearDepthDepthStencilState = VuGfx::IF()->createDepthStencilState(dssParams);

		dssParams.reset();
		dssParams.mDepthCompFunc = VUGFX_COMP_ALWAYS;
		dssParams.mDepthWriteEnabled = false;
		mpClearNoDepthDepthStencilState = VuGfx::IF()->createDepthStencilState(dssParams);
	}

	// clear color/depth pipeline states
	{
		VuGfxSortMaterial *pMat = mpBasicShaders->get2dXyzMaterial(VuBasicShaders::FLV_OPAQUE);
		VuShaderProgram *pSP = pMat->mpShaderProgram;
		VuVertexDeclaration *pVD = pMat->mpPipelineState->mpVertexDeclaration;

		VuPipelineStateParams params;
		mpClearColorPipelineState = VuGfx::IF()->createPipelineState(pSP, pVD, params);

		params.mColorWriteEnabled = false;
		mpClearNoColorPipelineState = VuGfx::IF()->createPipelineState(pSP, pVD, params);
	}

	return true;
}

//*****************************************************************************
void VuGfxUtil::release()
{
	mpBasicShaders->release();
	mpCollisionShader->release();
	mpDepthShader->release();
	mpShadowShader->release();
	mpBlobShadowShader->release();
	mpDropShadowShader->release();
	mpFontDraw->release();
	mpPostProcess->release();
	mpWhiteTexture->removeRef();
	mpBlackVertexColors->removeRef();
	mpDefaultDepthStencilState->removeRef();
	mpLessEqualDepthStencilState->removeRef();
	mpPostProcessDepthStencilState->removeRef();
	mpClearColorPipelineState->removeRef();
	mpClearNoColorPipelineState->removeRef();
	mpClearDepthDepthStencilState->removeRef();
	mpClearNoDepthDepthStencilState->removeRef();
}

//*****************************************************************************
bool VuGfxUtil::setDefaultRenderState()
{
	// render state
	VuGfx::IF()->setDepthStencilState(mpDefaultDepthStencilState);
	VuGfx::IF()->setCullMode(VUGFX_CULL_CW);
	for ( int i = 0; i < 8; i++ )
		VuGfx::IF()->setTexture(i, VUNULL);

	return true;
}

//*****************************************************************************
void VuGfxUtil::pushMatrix(const VuMatrix &mat)
{
	mMatrixStack.push(mat);
}

//*****************************************************************************
void VuGfxUtil::popMatrix()
{
	VUASSERT(mMatrixStack.size() > 1, "VuGfxUtil::popMatrix() extra pop");
	mMatrixStack.pop();
}

//*****************************************************************************
const VuMatrix &VuGfxUtil::getMatrix()
{
	return mMatrixStack.top();
}

//*****************************************************************************
void VuGfxUtil::pushTextScale(float scale)
{
	mTextScaleStack.push(scale);
}

//*****************************************************************************
void VuGfxUtil::popTextScale()
{
	VUASSERT(mTextScaleStack.size() > 1, "VuGfxUtil::popTextScale() extra pop");
	mTextScaleStack.pop();
}

//*****************************************************************************
float VuGfxUtil::getTextScale()
{
	return mTextScaleStack.top();
}

//*****************************************************************************
void VuGfxUtil::drawTexture2d(float depth, VuTexture *pTexture, const VuColor &color, const VuRect &dstRect, const VuRect &srcRect, VUUINT transType)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			float fx0 = pData->mDstRect.getLeft();
			float fx1 = pData->mDstRect.getRight();
			float fy0 = pData->mDstRect.getTop();
			float fy1 = pData->mDstRect.getBottom();

			float fu0 = pData->mSrcRect.getLeft();
			float fu1 = pData->mSrcRect.getRight();
			float fv0 = pData->mSrcRect.getTop();
			float fv1 = pData->mSrcRect.getBottom();

			VuGfxUtil::IF()->basicShaders()->set2dXyzUvConstants(pData->mTransform, pData->mColor);
			VuGfxUtil::IF()->basicShaders()->set2dXyzUvTexture(pData->mpTexture);

			VuVertex2dXyzUv verts[4];
			verts[0].mXyz[0] = fx0; verts[0].mXyz[1] = fy0; verts[0].mXyz[2] = pData->mDepth; verts[0].mUv[0] = fu0; verts[0].mUv[1] = fv0;
			verts[1].mXyz[0] = fx0; verts[1].mXyz[1] = fy1; verts[1].mXyz[2] = pData->mDepth; verts[1].mUv[0] = fu0; verts[1].mUv[1] = fv1;
			verts[2].mXyz[0] = fx1; verts[2].mXyz[1] = fy0; verts[2].mXyz[2] = pData->mDepth; verts[2].mUv[0] = fu1; verts[2].mUv[1] = fv0;
			verts[3].mXyz[0] = fx1; verts[3].mXyz[1] = fy1; verts[3].mXyz[2] = pData->mDepth; verts[3].mUv[0] = fu1; verts[3].mUv[1] = fv1;
			
			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
		}

		float		mDepth;
		VuTexture	*mpTexture;
		VuMatrix	mTransform;
		VuColor		mColor;
		VuRect		mSrcRect;
		VuRect		mDstRect;
	};

	if ( pTexture == VUNULL )
		pTexture = whiteTexture();
	
	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mDepth = depth;
	pData->mpTexture = pTexture;;
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mSrcRect = srcRect;
	pData->mDstRect = dstRect;

	VuBasicShaders::eFlavor flavor = sBasicShaderFlavorLookup[transType];
	VuGfxSort::IF()->submitDrawCommand<true>(transType, basicShaders()->get2dXyzUvMaterial(flavor), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawMaskedTexture2d(float depth, VuTexture *pTexture, VuTexture *pMaskTexture, const VuColor &color, const VuRect &dstRect, const VuRect &srcRect)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			float fx0 = pData->mDstRect.getLeft();
			float fx1 = pData->mDstRect.getRight();
			float fy0 = pData->mDstRect.getTop();
			float fy1 = pData->mDstRect.getBottom();

			float fu0 = pData->mSrcRect.getLeft();
			float fu1 = pData->mSrcRect.getRight();
			float fv0 = pData->mSrcRect.getTop();
			float fv1 = pData->mSrcRect.getBottom();

			VuGfxUtil::IF()->basicShaders()->set2dXyzUvMaskConstants(pData->mTransform, pData->mColor);
			VuGfxUtil::IF()->basicShaders()->set2dXyzUvMaskTextures(pData->mpTexture, pData->mpMaskTexture);

			VuVertex2dXyzUv verts[4];
			verts[0].mXyz[0] = fx0; verts[0].mXyz[1] = fy0; verts[0].mXyz[2] = pData->mDepth; verts[0].mUv[0] = fu0; verts[0].mUv[1] = fv0;
			verts[1].mXyz[0] = fx0; verts[1].mXyz[1] = fy1; verts[1].mXyz[2] = pData->mDepth; verts[1].mUv[0] = fu0; verts[1].mUv[1] = fv1;
			verts[2].mXyz[0] = fx1; verts[2].mXyz[1] = fy0; verts[2].mXyz[2] = pData->mDepth; verts[2].mUv[0] = fu1; verts[2].mUv[1] = fv0;
			verts[3].mXyz[0] = fx1; verts[3].mXyz[1] = fy1; verts[3].mXyz[2] = pData->mDepth; verts[3].mUv[0] = fu1; verts[3].mUv[1] = fv1;
			
			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
		}

		float		mDepth;
		VuTexture *	mpTexture;
		VuTexture *	mpMaskTexture;
		VuMatrix	mTransform;
		VuColor		mColor;
		VuRect		mSrcRect;
		VuRect		mDstRect;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mDepth = depth;
	pData->mpTexture = pTexture;
	pData->mpMaskTexture = pMaskTexture;
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mSrcRect = srcRect;
	pData->mDstRect = dstRect;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzUvMaskMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawFilledRectangle2d(float depth, const VuColor &color, const VuRect &rect)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			float fx0 = pData->mRect.getLeft();
			float fx1 = pData->mRect.getRight();
			float fy0 = pData->mRect.getTop();
			float fy1 = pData->mRect.getBottom();

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(pData->mTransform, pData->mColor);

			VuVertex2dXyz verts[4];
			verts[0].mXyz[0] = fx0; verts[0].mXyz[1] = fy0; verts[0].mXyz[2] = pData->mDepth;
			verts[1].mXyz[0] = fx0; verts[1].mXyz[1] = fy1; verts[1].mXyz[2] = pData->mDepth;
			verts[2].mXyz[0] = fx1; verts[2].mXyz[1] = fy0; verts[2].mXyz[2] = pData->mDepth;
			verts[3].mXyz[0] = fx1; verts[3].mXyz[1] = fy1; verts[3].mXyz[2] = pData->mDepth;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
		}

		float		mDepth;
		VuMatrix	mTransform;
		VuColor		mColor;
		VuRect		mRect;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mDepth = depth;
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mRect = rect;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawFilledTriangle2d(float depth, const VuColor &color, const VuVector2 &p0, const VuVector2 &p1, const VuVector2 &p2)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(pData->mTransform, pData->mColor);

			VuVertex2dXyz verts[3];
			verts[0].mXyz[0] = pData->mP0.mX; verts[0].mXyz[1] = pData->mP0.mY; verts[0].mXyz[2] = 0.0f;
			verts[1].mXyz[0] = pData->mP1.mX; verts[1].mXyz[1] = pData->mP1.mY; verts[1].mXyz[2] = 0.0f;
			verts[2].mXyz[0] = pData->mP2.mX; verts[2].mXyz[1] = pData->mP2.mY; verts[2].mXyz[2] = 0.0f;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLELIST, 1, verts);
		}

		VuMatrix	mTransform;
		VuColor		mColor;
		VuVector2	mP0, mP1, mP2;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mP0 = p0;
	pData->mP1 = p1;
	pData->mP2 = p2;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawRectangleOutline2d(float depth, const VuColor &color, const VuRect &rect)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			float fx0 = pData->mRect.getLeft();
			float fx1 = pData->mRect.getRight();
			float fy0 = pData->mRect.getTop();
			float fy1 = pData->mRect.getBottom();

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(pData->mTransform, pData->mColor);

			VuVertex2dXyz verts[5];
			verts[0].mXyz[0] = fx0; verts[0].mXyz[1] = fy0; verts[0].mXyz[2] = pData->mDepth;
			verts[1].mXyz[0] = fx1; verts[1].mXyz[1] = fy0; verts[1].mXyz[2] = pData->mDepth;
			verts[2].mXyz[0] = fx1; verts[2].mXyz[1] = fy1; verts[2].mXyz[2] = pData->mDepth;
			verts[3].mXyz[0] = fx0; verts[3].mXyz[1] = fy1; verts[3].mXyz[2] = pData->mDepth;
			verts[4].mXyz[0] = fx0; verts[4].mXyz[1] = fy0; verts[4].mXyz[2] = pData->mDepth;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINESTRIP, 4, verts);
		}

		float		mDepth;
		VuMatrix	mTransform;
		VuColor		mColor;
		VuRect		mRect;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mDepth = depth;
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mRect = rect;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawEllipseOutline2d(float depth, const VuColor &color, const VuRect &rect, int numSegments)
{
	if ( numSegments <= 0 )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(pData->mTransform, pData->mColor);

			VuVector2 vCenter = pData->mRect.getCenter();
			VuVector2 vRadius = 0.5f*pData->mRect.getSize();

			float fStep = 2.0f*VU_PI/pData->mNumSegments;
			float fCurAngle = 0.0f;
			float fNextAngle = fStep;
			for ( int iSegment = 0; iSegment < pData->mNumSegments; iSegment++ )
			{
				VuVector2 v0 = vCenter + vRadius*VuVector2(VuCos(fCurAngle), VuSin(fCurAngle));
				VuVector2 v1 = vCenter + vRadius*VuVector2(VuCos(fNextAngle), VuSin(fNextAngle));

				VuVertex2dXyz verts[2];
				verts[0].mXyz[0] = v0.mX; verts[0].mXyz[1] = v0.mY; verts[0].mXyz[2] = 0.0f;
				verts[1].mXyz[0] = v1.mX; verts[1].mXyz[1] = v1.mY; verts[1].mXyz[2] = 0.0f;

				VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, 1, verts);

				fCurAngle = fNextAngle;
				fNextAngle += fStep;
			}
		}

		VuMatrix	mTransform;
		VuColor		mColor;
		VuRect		mRect;
		int			mNumSegments;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mRect = rect;
	pData->mNumSegments = numSegments;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawLine2d(float depth, const VuColor &color, const VuVector2 &p0, const VuVector2 &p1)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(pData->mTransform, pData->mColor);

			VuVertex2dXyz verts[2];
			verts[0].mXyz[0] = pData->mP0.mX; verts[0].mXyz[1] = pData->mP0.mY; verts[0].mXyz[2] = 0.0f;
			verts[1].mXyz[0] = pData->mP1.mX; verts[1].mXyz[1] = pData->mP1.mY; verts[1].mXyz[2] = 0.0f;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, 1, verts);
		}

		VuMatrix	mTransform;
		VuColor		mColor;
		VuVector2	mP0, mP1;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mTransform = getMatrix();
	pData->mColor = color;
	pData->mP0 = p0;
	pData->mP1 = p1;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawLine2d(float depth, const VuVector2 &p0, const VuColor &col0, const VuVector2 &p1, const VuColor &col1)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set2dXyzColConstants(pData->mTransform);

			VuVertex2dXyzCol verts[2];
			verts[0].mXyz[0] = pData->mP0.mX; verts[0].mXyz[1] = pData->mP0.mY; verts[0].mXyz[2] = 0.0f; verts[0].mColor = pData->mCol0;
			verts[1].mXyz[0] = pData->mP1.mX; verts[1].mXyz[1] = pData->mP1.mY; verts[1].mXyz[2] = 0.0f; verts[1].mColor = pData->mCol1;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, 1, verts);
		}

		VuMatrix	mTransform;
		VuVector2	mP0, mP1;
		VuColor		mCol0, mCol1;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mTransform = getMatrix();
	pData->mP0 = p0;
	pData->mP1 = p1;
	pData->mCol0 = col0;
	pData->mCol1 = col1;

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzColMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawLines2d(float depth, VuGfxPrimitiveType type, const VuColor &color, const VuVector2 *pVerts, int vertCount)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);
			VuPackedVector3 *pVertData = reinterpret_cast<VuPackedVector3 *>(pData + 1);

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(pData->mTransform, pData->mColor);

			int primCount = VuVertexCountToPrimitiveCount(pData->mType, pData->mVertCount);

			VuGfx::IF()->drawPrimitiveUP(pData->mType, primCount, pVertData);
		}

		VuMatrix			mTransform;
		VuGfxPrimitiveType	mType;
		VuColor				mColor;
		int					mVertCount;
	};

	int allocSize = sizeof(DrawData) + vertCount*sizeof(VuPackedVector3);
	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(allocSize));

	pData->mTransform = getMatrix();
	pData->mType = type;
	pData->mColor = color;
	pData->mVertCount = vertCount;

	VuPackedVector3 *pVertData = reinterpret_cast<VuPackedVector3 *>(pData + 1);
	for ( int i = 0; i < vertCount; i++ )
	{
		pVertData[i].mX = pVerts[i].mX;
		pVertData[i].mY = pVerts[i].mY;
		pVertData[i].mZ = 0.0f;
	}

	VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_MODULATE, basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback, depth);
}

//*****************************************************************************
void VuGfxUtil::drawAxisInfo(float depth, const VuMatrix &orientation, const VuRect &rect)
{
	// calculate camera axes in screen space
	VuVector2 x(orientation.getAxisX().mX, -orientation.getAxisX().mY);
	VuVector2 y(orientation.getAxisY().mX, -orientation.getAxisY().mY);
	VuVector2 z(orientation.getAxisZ().mX, -orientation.getAxisZ().mY);

	VuMatrix mat = getMatrix();
	mat.translate(VuVector3(1.0f, 1.0f, 0.0f));
	mat.scale(VuVector3(0.5f, 0.5f, 1.0f));
	mat.scale(VuVector3(rect.getWidth(), rect.getHeight(), 1.0f));
	mat.translate(VuVector3(rect.getLeft(), rect.getTop(), 0.0f));
	pushMatrix(mat);
	{
		drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0,0,0,64), VuRect(-1, -1, 2, 2));
		drawLine2d(0, VuColor(255, 0, 0), VuVector2(0,0), x);
		drawLine2d(0, VuColor(0, 255, 0), VuVector2(0,0), y);
		drawLine2d(0, VuColor(0, 0, 255), VuVector2(0,0), z);
	}
	popMatrix();
}

//*****************************************************************************
void VuGfxUtil::drawLine3d(const VuColor &color, const VuVector3 &p0, const VuVector3 &p1, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			VuVertex3dXyz verts[2];
			verts[0].mXyz[0] = pData->mP0.mX; verts[0].mXyz[1] = pData->mP0.mY; verts[0].mXyz[2] = pData->mP0.mZ;
			verts[1].mXyz[0] = pData->mP1.mX; verts[1].mXyz[1] = pData->mP1.mY; verts[1].mXyz[2] = pData->mP1.mZ;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, 1, verts);
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		VuVector3	mP0, mP1;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mP0 = p0;
	pData->mP1 = p1;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawLine3d(const VuVector3 &p0, const VuColor &col0, const VuVector3 &p1, const VuColor &col1, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzColConstants(pData->mModelViewProjMat);

			VuVertex3dXyzCol verts[2];
			verts[0].mXyz[0] = pData->mP0.mX; verts[0].mXyz[1] = pData->mP0.mY; verts[0].mXyz[2] = pData->mP0.mZ; verts[0].mColor = pData->mCol0;
			verts[1].mXyz[0] = pData->mP1.mX; verts[1].mXyz[1] = pData->mP1.mY; verts[1].mXyz[2] = pData->mP1.mZ; verts[1].mColor = pData->mCol1;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, 1, verts);
		}

		VuMatrix	mModelViewProjMat;
		VuVector3	mP0, mP1;
		VuColor		mCol0, mCol1;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mP0 = p0;
	pData->mP1 = p1;
	pData->mCol0 = col0;
	pData->mCol1 = col1;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzColMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawLines3d(VuGfxPrimitiveType type, const VuColor &color, const VuVector3 *pVerts, int vertCount, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);
			VuPackedVector3 *pVertData = reinterpret_cast<VuPackedVector3 *>(pData + 1);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			int primCount = VuVertexCountToPrimitiveCount(pData->mType, pData->mVertCount);

			VuGfx::IF()->drawPrimitiveUP(pData->mType, primCount, pVertData);
		}

		VuMatrix			mModelViewProjMat;
		VuGfxPrimitiveType	mType;
		VuColor				mColor;
		int					mVertCount;
	};

	int allocSize = sizeof(DrawData) + vertCount*sizeof(VuPackedVector3);
	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(allocSize));

	pData->mModelViewProjMat = modelViewProjMat;
	pData->mType = type;
	pData->mColor = color;
	pData->mVertCount = vertCount;

	VuPackedVector3 *pVertData = reinterpret_cast<VuPackedVector3 *>(pData + 1);
	for ( int i = 0; i < vertCount; i++ )
	{
		pVertData[i].mX = pVerts[i].mX;
		pVertData[i].mY = pVerts[i].mY;
		pVertData[i].mZ = pVerts[i].mZ;
	}

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawAabbLines(const VuColor &color, const VuAabb &aabb, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			VuPackedVector3 verts[8];
			pData->mAabb.getVerts(verts);

			const VUUINT16 *indices = pData->mAabb.getEdgeIndices();

			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_LINELIST,		// PrimitiveType
				0,						// MinVertexIndex
				8,						// NumVertices
				12,						// PrimitiveCount
				indices,				// IndexData
				verts					// VertexStreamZeroData
			);
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		VuAabb		mAabb;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mAabb = aabb;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawCylinderLines(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelViewProjMat)
{
	if ( numSides <= 0 )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			float fStep = 2.0f*VU_PI/pData->mNumSides;
			float fCurAngle = 0.0f;
			float fNextAngle = fStep;
			for ( int iSide = 0; iSide < pData->mNumSides; iSide++ )
			{
				VuVector2 v0 = pData->mRadius*VuVector2(VuCos(fCurAngle), VuSin(fCurAngle));
				VuVector2 v1 = pData->mRadius*VuVector2(VuCos(fNextAngle), VuSin(fNextAngle));

				VuVertex3dXyz verts[4];
				verts[0].mXyz[0] = v0.mX; verts[0].mXyz[1] = v0.mY; verts[0].mXyz[2] = -0.5f*pData->mHeight;
				verts[1].mXyz[0] = v1.mX; verts[1].mXyz[1] = v1.mY; verts[1].mXyz[2] = -0.5f*pData->mHeight;
				verts[2].mXyz[0] = v1.mX; verts[2].mXyz[1] = v1.mY; verts[2].mXyz[2] = 0.5f*pData->mHeight;
				verts[3].mXyz[0] = v0.mX; verts[3].mXyz[1] = v0.mY; verts[3].mXyz[2] = 0.5f*pData->mHeight;

				VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINESTRIP, 3, verts);

				fCurAngle = fNextAngle;
				fNextAngle += fStep;
			}
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		float		mHeight;
		float		mRadius;
		int			mNumSides;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mHeight = fHeight;
	pData->mRadius = fRadius;
	pData->mNumSides = numSides;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawSphereLines(const VuColor &color, float fRadius, int axisSubdivCount, int heightSubdivCount, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			// use scratch pad to build verts
			VuVertex3dXyz *verts = (VuVertex3dXyz *)VuScratchPad::get(VuScratchPad::GRAPHICS);
			int vertCount = 0;

			// draw horizontal lines (latitudes)
			for ( int i = 1; i < pData->mHeightSubdivCount; i++ )
			{
				float thi = VU_PI*i/pData->mHeightSubdivCount;
				float sinThi, cosThi;
				VuSinCosEst(thi, sinThi, cosThi);
				float theta = 0, step = VU_2PI/pData->mAxisSubdivCount;
				for ( int j = 0; j < pData->mAxisSubdivCount; j++ )
				{
					verts[vertCount].mXyz[0] = pData->mRadius*sinThi*VuCosEst(theta);
					verts[vertCount].mXyz[1] = pData->mRadius*sinThi*VuSinEst(theta);
					verts[vertCount].mXyz[2] = pData->mRadius*cosThi;
					vertCount++;

					verts[vertCount].mXyz[0] = pData->mRadius*sinThi*VuCosEst(theta + step);
					verts[vertCount].mXyz[1] = pData->mRadius*sinThi*VuSinEst(theta + step);
					verts[vertCount].mXyz[2] = pData->mRadius*cosThi;
					vertCount++;

					theta += step;
				}
			}

			// draw vertical lines (longitudes)
			for ( int i = 0; i < pData->mAxisSubdivCount; i++ )
			{
				float theta = VU_2PI*i/pData->mAxisSubdivCount;
				float sinTheta, cosTheta;
				VuSinCosEst(theta, sinTheta, cosTheta);
				float thi = 0, step = VU_PI/pData->mHeightSubdivCount;
				for ( int j = 0; j < pData->mHeightSubdivCount; j++ )
				{
					verts[vertCount].mXyz[0] = pData->mRadius*VuSinEst(thi)*cosTheta;
					verts[vertCount].mXyz[1] = pData->mRadius*VuSinEst(thi)*sinTheta;
					verts[vertCount].mXyz[2] = pData->mRadius*VuCosEst(thi);
					vertCount++;

					verts[vertCount].mXyz[0] = pData->mRadius*VuSinEst(thi + step)*cosTheta;
					verts[vertCount].mXyz[1] = pData->mRadius*VuSinEst(thi + step)*sinTheta;
					verts[vertCount].mXyz[2] = pData->mRadius*VuCosEst(thi + step);
					vertCount++;

					thi += step;
				}
			}

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, vertCount/2, verts);
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		float		mRadius;
		int			mAxisSubdivCount;
		int			mHeightSubdivCount;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mRadius = fRadius;
	pData->mAxisSubdivCount = axisSubdivCount;
	pData->mHeightSubdivCount = heightSubdivCount;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawConeLines(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelViewProjMat)
{
	if ( numSides <= 0 )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			float fStep = 2.0f*VU_PI/pData->mNumSides;
			float fCurAngle = 0.0f;
			float fNextAngle = fStep;
			for ( int iSide = 0; iSide < pData->mNumSides; iSide++ )
			{
				VuVector2 v0 = pData->mRadius*VuVector2(VuCos(fCurAngle), VuSin(fCurAngle));
				VuVector2 v1 = pData->mRadius*VuVector2(VuCos(fNextAngle), VuSin(fNextAngle));

				VuVertex3dXyz verts[3];
				verts[0].mXyz[0] = v0.mX; verts[0].mXyz[1] = v0.mY; verts[0].mXyz[2] = 0.0f;
				verts[1].mXyz[0] = v1.mX; verts[1].mXyz[1] = v1.mY; verts[1].mXyz[2] = 0.0f;
				verts[2].mXyz[0] = 0;     verts[2].mXyz[1] = 0;     verts[2].mXyz[2] = pData->mHeight;

				VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINESTRIP, 2, verts);

				fCurAngle = fNextAngle;
				fNextAngle += fStep;
			}
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		float		mHeight;
		float		mRadius;
		int			mNumSides;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mHeight = fHeight;
	pData->mRadius = fRadius;
	pData->mNumSides = numSides;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawArrowLines(const VuColor &color, float fLength, float fHeadLength, float fHeadWidth, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			VuVertex3dXyz verts[5];

			verts[0].mXyz[0] = 0;                  verts[0].mXyz[1] = 0;                                   verts[0].mXyz[2] = 0; // base
			verts[1].mXyz[0] = 0;                  verts[1].mXyz[1] = pData->mLength;                      verts[1].mXyz[2] = 0; // tip
			verts[2].mXyz[0] = 0;                  verts[2].mXyz[1] = pData->mLength - pData->mHeadLength; verts[2].mXyz[2] = 0; // head base
			verts[3].mXyz[0] = -pData->mHeadWidth; verts[3].mXyz[1] = pData->mLength - pData->mHeadLength; verts[3].mXyz[2] = 0; // head left
			verts[4].mXyz[0] =  pData->mHeadWidth; verts[4].mXyz[1] = pData->mLength - pData->mHeadLength; verts[4].mXyz[2] = 0; // head right

			VUUINT16 indices[8] =
			{
				0, 2, // shaft
				3, 4, // head base
				1, 3, // head left
				1, 4, // head right
			};

			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_LINELIST,		// PrimitiveType
				0,						// MinVertexIndex
				5,						// NumVertices
				4,						// PrimitiveCount
				indices,				// IndexData
				verts					// VertexStreamZeroData
			);
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		float		mLength;
		float		mHeadLength;
		float		mHeadWidth;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mLength = fLength;
	pData->mHeadLength = fHeadLength;
	pData->mHeadWidth = fHeadWidth;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawArcLines(const VuColor &color, const VuVector3 &pos, const VuVector3 &axis, const VuVector3 &right, float minAngle, float maxAngle, float radius, int segmentCount, bool drawSect, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			VuVector3 fwd = VuCross(pData->mAxis, pData->mRight).normal();

			// use scratch pad to build verts
			VuVertex3dXyz *verts = (VuVertex3dXyz *)VuScratchPad::get(VuScratchPad::GRAPHICS);
			VuVertex3dXyz *pv = verts;

			VuMatrix mat;
			mat.loadIdentity();
			mat.setAxisX(pData->mRight);
			mat.setAxisY(fwd);
			mat.setAxisZ(pData->mAxis);
			mat.setTrans(pData->mPos);

//			VuVector3 prevPos = mat.transform(VuVector3(pData->mRadius, 0.0f, 0.0f));
			float radiansPerSegment = (pData->mMaxAngle - pData->mMinAngle) / pData->mSegmentCount;

			mat.rotateZLocal(pData->mMinAngle);
			if ( pData->mDrawSect )
			{
				pv->mXyz[0] = pData->mPos.mX;
				pv->mXyz[1] = pData->mPos.mY;
				pv->mXyz[2] = pData->mPos.mZ;
				pv++;
			}
			for(int iSegment = 0; iSegment <= pData->mSegmentCount; iSegment++)
			{
				VuVector3 curPos = mat.transform(VuVector3(pData->mRadius, 0.0f, 0.0f));
				pv->mXyz[0] = curPos.mX;
				pv->mXyz[1] = curPos.mY;
				pv->mXyz[2] = curPos.mZ;
				pv++;
				mat.rotateZLocal(radiansPerSegment);
			}
			if ( pData->mDrawSect )
			{
				pv->mXyz[0] = pData->mPos.mX;
				pv->mXyz[1] = pData->mPos.mY;
				pv->mXyz[2] = pData->mPos.mZ;
				pv++;
			}

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINESTRIP, pData->mSegmentCount + pData->mDrawSect*2, verts);
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		VuVector3	mPos;
		VuVector3	mAxis;
		VuVector3	mRight;
		float		mMinAngle;
		float		mMaxAngle;
		float		mRadius;
		int			mSegmentCount;
		bool		mDrawSect;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mPos = pos;
	pData->mAxis = axis;
	pData->mRight = right;
	pData->mMinAngle = minAngle;
	pData->mMaxAngle = maxAngle;
	pData->mRadius = radius;
	pData->mSegmentCount = segmentCount;
	pData->mDrawSect = drawSect;
	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::drawTriangleStrip(const VuColor &color, const VuVector3 *pVerts, int vertCount, const VuMatrix &modelViewProjMat)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);
			VuPackedVector3 *pVertData = reinterpret_cast<VuPackedVector3 *>(pData + 1);

			VuGfxUtil::IF()->basicShaders()->set3dXyzConstants(pData->mModelViewProjMat, pData->mColor);

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, pData->mVertCount - 2, pVertData);
		}

		VuMatrix	mModelViewProjMat;
		VuColor		mColor;
		int			mVertCount;
	};

	int allocSize = sizeof(DrawData) + vertCount*sizeof(VuPackedVector3);
	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(allocSize));

	pData->mModelViewProjMat = modelViewProjMat;
	pData->mColor = color;
	pData->mVertCount = vertCount;

	VuPackedVector3 *pVertData = reinterpret_cast<VuPackedVector3 *>(pData + 1);
	for ( int i = 0; i < vertCount; i++ )
	{
		pVertData[i].mX = pVerts[i].mX;
		pVertData[i].mY = pVerts[i].mY;
		pVertData[i].mZ = pVerts[i].mZ;
	}

	if ( color.mA == 255 )
	{
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
	}
	else
	{
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback);
	}
}

//*****************************************************************************
void VuGfxUtil::drawAabbSolid(const VuColor &color, const VuAabb &aabb, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(pData->mModelMat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

			VuPackedVector3 verts[48];
			pData->mAabb.getPosNorVerts(verts);

			const VUUINT16 *indices = pData->mAabb.getPosNorTriIndices();

			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,	// PrimitiveType
				0,						// MinVertexIndex
				24,						// NumVertices
				12,						// PrimitiveCount
				indices,				// IndexData
				verts					// VertexStreamZeroData
			);
		}

		VuMatrix	mModelMat;
		VuMatrix	mViewProjMat;
		VuColor		mColor;
		VuAabb		mAabb;
		VuVector3	mDirLight;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mViewProjMat = viewProjMat;
	pData->mColor = color;
	pData->mAabb = aabb;
	pData->mDirLight = dirLight;

	if ( color.mA == 255 )
	{
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
	}
	else
	{
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback);
	}
}

//*****************************************************************************
void VuGfxUtil::drawCylinderSolid(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight)
{
	if ( numSides <= 0 )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(pData->mModelMat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

			// calculate vert & tri counts
			int vertCount = 2 + 2*pData->mNumSides + 2*pData->mNumSides;
			int triCount = 2*pData->mNumSides + 2*pData->mNumSides;

			// use scratch pad to build verts/tris
			VuVertex3dXyzNor *verts = (VuVertex3dXyzNor *)VuScratchPad::get(VuScratchPad::GRAPHICS);
			VuVertex3dXyzNor *pv = verts;

			// top
			pv->mXyz[0] = 0.0f; pv->mXyz[1] = 0.0f; pv->mXyz[2] = 0.5f*pData->mHeight;
			pv->mNor[0] = 0.0f; pv->mNor[1] = 0.0f; pv->mNor[2] = 1.0f;
			pv++;

			float theta = 0, step = VU_2PI/pData->mNumSides;
			for ( int i = 0; i < pData->mNumSides; i++ )
			{
				float x, y;
				VuSinCosEst(theta, y, x);
				pv->mXyz[0] = pData->mRadius*x; pv->mXyz[1] = pData->mRadius*y; pv->mXyz[2] = 0.5f*pData->mHeight;
				pv->mNor[0] = 0.0f;             pv->mNor[1] = 0.0f;             pv->mNor[2] = 1.0f;
				pv++;

				theta += step;
			}

			// sides
			theta = 0;
			for ( int i = 0; i < pData->mNumSides; i++ )
			{
				float x, y;
				VuSinCosEst(theta, y, x);

				pv->mXyz[0] = pData->mRadius*x; pv->mXyz[1] = pData->mRadius*y; pv->mXyz[2] = 0.5f*pData->mHeight;
				pv->mNor[0] = x;                pv->mNor[1] = y;                pv->mNor[2] = 0.0f;
				pv++;

				pv->mXyz[0] = pData->mRadius*x; pv->mXyz[1] = pData->mRadius*y; pv->mXyz[2] = -0.5f*pData->mHeight;
				pv->mNor[0] = x;                pv->mNor[1] = y;                pv->mNor[2] = 0.0f;
				pv++;

				theta += step;
			}

			// bottom
			theta = 0;
			for ( int i = 0; i < pData->mNumSides; i++ )
			{
				float x, y;
				VuSinCosEst(theta, y, x);
				pv->mXyz[0] = pData->mRadius*x; pv->mXyz[1] = pData->mRadius*y; pv->mXyz[2] = -0.5f*pData->mHeight;
				pv->mNor[0] = 0.0f;             pv->mNor[1] = 0.0f;             pv->mNor[2] = -1.0f;
				pv++;

				theta += step;
			}

			pv->mXyz[0] = 0.0f; pv->mXyz[1] = 0.0f; pv->mXyz[2] = -0.5f*pData->mHeight;
			pv->mNor[0] = 0.0f; pv->mNor[1] = 0.0f; pv->mNor[2] = -1.0f;
			pv++;


			// use scratch pad to build indices
			VUUINT16 *inds = (VUUINT16 *)(verts + vertCount);
			VUUINT16 *pi = inds;

			#define ADD_TRI(i0, i1, i2)	\
			{							\
				(*pi++) = (VUUINT16)(i0);	\
				(*pi++) = (VUUINT16)(i1);	\
				(*pi++) = (VUUINT16)(i2);	\
			}

			// top
			for ( int i = 0; i < pData->mNumSides - 1; i++ )
				ADD_TRI(0, i + 1, i + 2);
			ADD_TRI(0, pData->mNumSides, 1);

			// sides
			int firstVert = (VUUINT16)(pData->mNumSides + 1);
			for ( int i = 0; i < pData->mNumSides - 1; i++ )
			{
				ADD_TRI(firstVert + 2*i, firstVert + 2*i + 1, firstVert + 2*i + 2);
				ADD_TRI(firstVert + 2*i + 2, firstVert + 2*i + 1, firstVert + 2*i + 3);
			}
			ADD_TRI(firstVert + 2*pData->mNumSides - 2, firstVert + 2*pData->mNumSides - 1, firstVert);
			ADD_TRI(firstVert, firstVert + 2*pData->mNumSides - 1, firstVert + 1);

			// bottom
			for ( int i = 0; i < pData->mNumSides - 1; i++ )
				ADD_TRI(vertCount - 1, vertCount - i - 2, vertCount - i - 3);
			ADD_TRI(vertCount - 1, vertCount - pData->mNumSides - 1, vertCount - 2);

			#undef ADD_TRI

			// draw
			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,	// PrimitiveType
				0,						// MinVertexIndex
				vertCount,				// NumVertices
				triCount,				// PrimitiveCount
				inds,					// IndexData
				verts					// VertexStreamZeroData
			);
		}

		VuMatrix	mModelMat;
		VuMatrix	mViewProjMat;
		VuColor		mColor;
		float		mHeight;
		float		mRadius;
		int			mNumSides;
		VuVector3	mDirLight;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mViewProjMat = viewProjMat;
	pData->mColor = color;
	pData->mHeight = fHeight;
	pData->mRadius = fRadius;
	pData->mNumSides = numSides;
	pData->mDirLight = dirLight;

	if ( color.mA == 255 )
	{
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
	}
	else
	{
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback);
	}
}

//*****************************************************************************
void VuGfxUtil::drawSphereSolid(const VuColor &color, float fRadius, int axisSubdivCount, int heightSubdivCount, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(pData->mModelMat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

			// calculate vert & tri counts
			int vertCount = 2 + (pData->mHeightSubdivCount - 1)*pData->mAxisSubdivCount;
			int triCount = 2*pData->mAxisSubdivCount + 2*(pData->mHeightSubdivCount - 2)*pData->mAxisSubdivCount;

			// use scratch pad to build verts
			VuVertex3dXyzNor *verts = (VuVertex3dXyzNor *)VuScratchPad::get(VuScratchPad::GRAPHICS);
			VuVertex3dXyzNor *pv = verts;

			// top
			pv->mXyz[0] = 0.0f; pv->mXyz[1] = 0.0f; pv->mXyz[2] = pData->mRadius;
			pv->mNor[0] = 0.0f; pv->mNor[1] = 0.0f; pv->mNor[2] = 1.0f;
			pv++;

			// latitudes
			for ( int i = 1; i < pData->mHeightSubdivCount; i++ )
			{
				float thi = VU_PI*i/pData->mHeightSubdivCount;
				float sinThi, cosThi;
				VuSinCosEst(thi, sinThi, cosThi);
				float theta = 0, step = VU_2PI/pData->mAxisSubdivCount;
				for ( int j = 0; j < pData->mAxisSubdivCount; j++ )
				{
					VuVector3 nor(sinThi*VuCosEst(theta), sinThi*VuSinEst(theta), cosThi);
					VuVector3 pos = pData->mRadius*nor;
					pv->mXyz[0] = pos.mX; pv->mXyz[1] = pos.mY; pv->mXyz[2] = pos.mZ;
					pv->mNor[0] = nor.mX; pv->mNor[1] = nor.mY; pv->mNor[2] = nor.mZ;
					pv++;

					theta += step;
				}
			}

			// bottom
			pv->mXyz[0] = 0.0f; pv->mXyz[1] = 0.0f; pv->mXyz[2] = -pData->mRadius;
			pv->mNor[0] = 0.0f; pv->mNor[1] = 0.0f; pv->mNor[2] = -1.0f;
			pv++;


			// use scratch pad to build indices
			VUUINT16 *inds = (VUUINT16 *)(verts + vertCount);
			VUUINT16 *pi = inds;

			#define ADD_TRI(i0, i1, i2)	\
			{							\
				(*pi++) = (VUUINT16)(i0);	\
				(*pi++) = (VUUINT16)(i1);	\
				(*pi++) = (VUUINT16)(i2);	\
			}

			// top
			for ( int i = 0; i < pData->mAxisSubdivCount - 1; i++ )
				ADD_TRI(0, i + 1, i + 2);
			ADD_TRI(0, pData->mAxisSubdivCount, 1);

			// latitudes
			for ( int i = 0; i < pData->mHeightSubdivCount - 2; i++ )
			{
				int firstVert = i*pData->mAxisSubdivCount + 1;
				for ( int j = 0; j < pData->mAxisSubdivCount - 1; j++ )
				{
					ADD_TRI(firstVert + j, firstVert + j + pData->mAxisSubdivCount, firstVert + j + 1);
					ADD_TRI(firstVert + j + 1, firstVert + j + pData->mAxisSubdivCount, firstVert + pData->mAxisSubdivCount + j + 1);
				}
				ADD_TRI(firstVert + pData->mAxisSubdivCount - 1, firstVert + 2*pData->mAxisSubdivCount - 1, firstVert);
				ADD_TRI(firstVert, firstVert + 2*pData->mAxisSubdivCount - 1, firstVert + pData->mAxisSubdivCount);
			}

			// bottom
			for ( int i = 0; i < pData->mAxisSubdivCount - 1; i++ )
				ADD_TRI(vertCount - 1, vertCount - i - 2, vertCount - i - 3);
			ADD_TRI(vertCount - 1, vertCount - pData->mAxisSubdivCount - 1, vertCount - 2);

			#undef ADD_TRI


			// draw
			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,	// PrimitiveType
				0,						// MinVertexIndex
				vertCount,				// NumVertices
				triCount,				// PrimitiveCount
				inds,					// IndexData
				verts					// VertexStreamZeroData
			);
		}

		VuMatrix	mModelMat;
		VuMatrix	mViewProjMat;
		VuColor		mColor;
		float		mRadius;
		int			mAxisSubdivCount;
		int			mHeightSubdivCount;
		VuVector3	mDirLight;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mViewProjMat = viewProjMat;
	pData->mColor = color;
	pData->mRadius = fRadius;
	pData->mAxisSubdivCount = axisSubdivCount;
	pData->mHeightSubdivCount = heightSubdivCount;
	pData->mDirLight = dirLight;

	if ( color.mA == 255 )
	{
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
	}
	else
	{
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback);
	}
}

//*****************************************************************************
void VuGfxUtil::drawConeSolid(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight)
{
	if ( numSides <= 0 )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(pData->mModelMat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

			float fStep = 2.0f*VU_PI/pData->mNumSides;
			float fCurAngle = 0.0f;
			float fNextAngle = fStep;
			for ( int iSide = 0; iSide < pData->mNumSides; iSide++ )
			{
				VuVector3 v0 = pData->mRadius*VuVector3(VuCos(fCurAngle), VuSin(fCurAngle), 0);
				VuVector3 v1 = pData->mRadius*VuVector3(VuCos(fNextAngle), VuSin(fNextAngle), 0);
				VuVector3 vt = VuVector3(0, 0, pData->mHeight);
				VuVector3 vb = VuVector3(0, 0, 0);
				VuVector3 n = VuCross(v0 - vt, v1 - vt).normal();

				VuVertex3dXyzNor verts[6];
				verts[0].mXyz[0] = v0.mX; verts[0].mXyz[1] = v0.mY; verts[0].mXyz[2] = v0.mZ; verts[0].mNor[0] = n.mX; verts[0].mNor[1] = n.mY; verts[0].mNor[2] = n.mZ;
				verts[1].mXyz[0] = v1.mX; verts[1].mXyz[1] = v1.mY; verts[1].mXyz[2] = v1.mZ; verts[1].mNor[0] = n.mX; verts[1].mNor[1] = n.mY; verts[1].mNor[2] = n.mZ;
				verts[2].mXyz[0] = vt.mX; verts[2].mXyz[1] = vt.mY; verts[2].mXyz[2] = vt.mZ; verts[2].mNor[0] = n.mX; verts[2].mNor[1] = n.mY; verts[2].mNor[2] = n.mZ;

				verts[3].mXyz[0] = v1.mX; verts[3].mXyz[1] = v1.mY; verts[3].mXyz[2] = v1.mZ; verts[3].mNor[0] = 0; verts[3].mNor[1] = 0; verts[3].mNor[2] = -1;
				verts[4].mXyz[0] = v0.mX; verts[4].mXyz[1] = v0.mY; verts[4].mXyz[2] = v0.mZ; verts[4].mNor[0] = 0; verts[4].mNor[1] = 0; verts[4].mNor[2] = -1;
				verts[5].mXyz[0] = vb.mX; verts[5].mXyz[1] = vb.mY; verts[5].mXyz[2] = vb.mZ; verts[5].mNor[0] = 0; verts[5].mNor[1] = 0; verts[5].mNor[2] = -1;

				VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLELIST, 2, verts);

				fCurAngle = fNextAngle;
				fNextAngle += fStep;
			}
		}

		VuMatrix	mModelMat;
		VuMatrix	mViewProjMat;
		VuColor		mColor;
		float		mHeight;
		float		mRadius;
		int			mNumSides;
		VuVector3	mDirLight;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mViewProjMat = viewProjMat;
	pData->mColor = color;
	pData->mHeight = fHeight;
	pData->mRadius = fRadius;
	pData->mNumSides = numSides;
	pData->mDirLight = dirLight;

	if ( color.mA == 255 )
	{
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
	}
	else
	{
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback);
	}
}

//*****************************************************************************
void VuGfxUtil::drawCapsuleSolid(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight)
{
	if ( numSides <= 0 )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			#define ADD_TRI(i0, i1, i2)	\
			{							\
				(*pi++) = (VUUINT16)(i0);			\
				(*pi++) = (VUUINT16)(i1);			\
				(*pi++) = (VUUINT16)(i2);			\
			}

			// draw cylinder
			{
				// calculate vert & tri counts
				int vertCount = 2*pData->mNumSides;
				int triCount = 2*pData->mNumSides;

				// use scratch pad to build verts/tris
				VuVertex3dXyzNor *verts = (VuVertex3dXyzNor *)VuScratchPad::get(VuScratchPad::GRAPHICS);
				VuVertex3dXyzNor *pv = verts;

				// sides
				float theta = 0, step = VU_2PI/pData->mNumSides;
				for ( int i = 0; i < pData->mNumSides; i++ )
				{
					float x, y;
					VuSinCosEst(theta, y, x);

					pv->mXyz[0] = pData->mRadius*x; pv->mXyz[1] = pData->mRadius*y; pv->mXyz[2] = 0.5f*pData->mHeight;
					pv->mNor[0] = x;                pv->mNor[1] = y;                pv->mNor[2] = 0.0f;
					pv++;

					pv->mXyz[0] = pData->mRadius*x; pv->mXyz[1] = pData->mRadius*y; pv->mXyz[2] = -0.5f*pData->mHeight;
					pv->mNor[0] = x;                pv->mNor[1] = y;                pv->mNor[2] = 0.0f;
					pv++;

					theta += step;
				}

				// use scratch pad to build indices
				VUUINT16 *inds = (VUUINT16 *)(verts + vertCount);
				VUUINT16 *pi = inds;


				// sides
				VUUINT16 firstVert = 0;
				for ( int i = 0; i < pData->mNumSides - 1; i++ )
				{
					ADD_TRI(firstVert + 2*i, firstVert + 2*i + 1, firstVert + 2*i + 2);
					ADD_TRI(firstVert + 2*i + 2, firstVert + 2*i + 1, firstVert + 2*i + 3);
				}
				ADD_TRI(firstVert + 2*pData->mNumSides - 2, firstVert + 2*pData->mNumSides - 1, firstVert);
				ADD_TRI(firstVert, firstVert + 2*pData->mNumSides - 1, firstVert + 1);

				// draw
				VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(pData->mModelMat, pData->mViewProjMat, pData->mDirLight, pData->mColor);
				VuGfx::IF()->drawIndexedPrimitiveUP(VUGFX_PT_TRIANGLELIST, 0, vertCount, triCount, inds, verts);
			}

			// draw caps
			{
				int heightSubdivCount = pData->mNumSides/2;
				int axisSubdivCount = pData->mNumSides;

				// calculate vert & tri counts
				int vertCount = 1 + heightSubdivCount*axisSubdivCount;
				int triCount = axisSubdivCount + 2*(heightSubdivCount - 1)*axisSubdivCount;

				// use scratch pad to build verts
				VuVertex3dXyzNor *verts = (VuVertex3dXyzNor *)VuScratchPad::get(VuScratchPad::GRAPHICS);
				VuVertex3dXyzNor *pv = verts;

				// top
				pv->mXyz[0] = 0.0f; pv->mXyz[1] = 0.0f; pv->mXyz[2] = pData->mRadius;
				pv->mNor[0] = 0.0f; pv->mNor[1] = 0.0f; pv->mNor[2] = 1.0f;
				pv++;

				// latitudes
				for ( int i = 1; i <= heightSubdivCount; i++ )
				{
					float thi = VU_PIDIV2*i/heightSubdivCount;
					float sinThi, cosThi;
					VuSinCosEst(thi, sinThi, cosThi);
					float theta = 0, step = VU_2PI/axisSubdivCount;
					for ( int j = 0; j < axisSubdivCount; j++ )
					{
						VuVector3 nor(sinThi*VuCosEst(theta), sinThi*VuSinEst(theta), cosThi);
						VuVector3 pos = pData->mRadius*nor;
						pv->mXyz[0] = pos.mX; pv->mXyz[1] = pos.mY; pv->mXyz[2] = pos.mZ;
						pv->mNor[0] = nor.mX; pv->mNor[1] = nor.mY; pv->mNor[2] = nor.mZ;
						pv++;

						theta += step;
					}
				}

				// use scratch pad to build indices
				VUUINT16 *inds = (VUUINT16 *)(verts + vertCount);
				VUUINT16 *pi = inds;

				// top
				for ( int i = 0; i < axisSubdivCount - 1; i++ )
					ADD_TRI(0, i + 1, i + 2);
				ADD_TRI(0, axisSubdivCount, 1);

				// latitudes
				for ( int i = 0; i < heightSubdivCount - 1; i++ )
				{
					int firstVert = i*axisSubdivCount + 1;
					for ( int j = 0; j < axisSubdivCount - 1; j++ )
					{
						ADD_TRI(firstVert + j, firstVert + j + axisSubdivCount, firstVert + j + 1);
						ADD_TRI(firstVert + j + 1, firstVert + j + axisSubdivCount, firstVert + axisSubdivCount + j + 1);
					}
					ADD_TRI(firstVert + axisSubdivCount - 1, firstVert + 2*axisSubdivCount - 1, firstVert);
					ADD_TRI(firstVert, firstVert + 2*axisSubdivCount - 1, firstVert + axisSubdivCount);
				}

				// draw top cap
				{
					VuMatrix mat = pData->mModelMat;
					mat.translateLocal(VuVector3(0.0f, 0.0f, 0.5f*pData->mHeight));
					VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(mat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

					VuGfx::IF()->drawIndexedPrimitiveUP(VUGFX_PT_TRIANGLELIST, 0, vertCount, triCount, inds, verts);
				}

				// draw bottom cap
				{
					VuMatrix mat = pData->mModelMat;
					mat.translateLocal(VuVector3(0.0f, 0.0f, -0.5f*pData->mHeight));
					mat.rotateXLocal(VU_PI);
					VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(mat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

					VuGfx::IF()->drawIndexedPrimitiveUP(VUGFX_PT_TRIANGLELIST, 0, vertCount, triCount, inds, verts);
				}
			}

			#undef ADD_TRI
		}

		VuMatrix	mModelMat;
		VuMatrix	mViewProjMat;
		VuColor		mColor;
		float		mHeight;
		float		mRadius;
		int			mNumSides;
		VuVector3	mDirLight;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mViewProjMat = viewProjMat;
	pData->mColor = color;
	pData->mHeight = fHeight;
	pData->mRadius = fRadius;
	pData->mNumSides = numSides;
	pData->mDirLight = dirLight;

	if ( color.mA == 255 )
	{
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);
	}
	else
	{
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzNorMaterial(VuBasicShaders::FLV_MODULATED), VUNULL, &DrawData::callback);
	}
}

//*****************************************************************************
/*
void VuGfxUtil::drawHemisphereSolid(const VuColor &color, float fRadius, int axisSubdivCount, int heightSubdivCount, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->basicShaders()->set3dXyzNorConstants(pData->mModelMat, pData->mViewProjMat, pData->mDirLight, pData->mColor);

			// calculate vert & tri counts
			int vertCount = 1 + pData->mHeightSubdivCount*pData->mAxisSubdivCount;
			int triCount = 1*pData->mAxisSubdivCount + 2*(pData->mHeightSubdivCount - 1)*pData->mAxisSubdivCount;

			// use scratch pad to build verts
			VuVertex3dXyzNor *verts = (VuVertex3dXyzNor *)VuScratchPad::get(VuScratchPad::GRAPHICS);
			VuVertex3dXyzNor *pv = verts;

			// top
			pv->mXyz[0] = 0.0f; pv->mXyz[1] = 0.0f; pv->mXyz[2] = pData->mRadius;
			pv->mNor[0] = 0.0f; pv->mNor[1] = 0.0f; pv->mNor[2] = 1.0f;
			pv++;

			// latitudes
			for ( int i = 1; i <= pData->mHeightSubdivCount; i++ )
			{
				float thi = VU_PIDIV2*i/pData->mHeightSubdivCount;
				float sinThi, cosThi;
				VuSinCosEst(thi, sinThi, cosThi);
				float theta = 0, step = VU_2PI/pData->mAxisSubdivCount;
				for ( int j = 0; j < pData->mAxisSubdivCount; j++ )
				{
					VuVector3 nor(sinThi*VuCosEst(theta), sinThi*VuSinEst(theta), cosThi);
					VuVector3 pos = pData->mRadius*nor;
					pv->mXyz[0] = pos.mX; pv->mXyz[1] = pos.mY; pv->mXyz[2] = pos.mZ;
					pv->mNor[0] = nor.mX; pv->mNor[1] = nor.mY; pv->mNor[2] = nor.mZ;
					pv++;

					theta += step;
				}
			}

			// use scratch pad to build indices
			VUUINT16 *inds = (VUUINT16 *)(verts + vertCount);
			VUUINT16 *pi = inds;

			#define ADD_TRI(i0, i1, i2)	\
			{							\
				(*pi++) = i0;			\
				(*pi++) = i1;			\
				(*pi++) = i2;			\
			}

			// top
			for ( int i = 0; i < pData->mAxisSubdivCount - 1; i++ )
				ADD_TRI(0, i + 1, i + 2);
			ADD_TRI(0, pData->mAxisSubdivCount, 1);

			// latitudes
			for ( int i = 0; i < pData->mHeightSubdivCount - 1; i++ )
			{
				VUUINT16 firstVert = i*pData->mAxisSubdivCount + 1;
				for ( int j = 0; j < pData->mAxisSubdivCount - 1; j++ )
				{
					ADD_TRI(firstVert + j, firstVert + j + pData->mAxisSubdivCount, firstVert + j + 1);
					ADD_TRI(firstVert + j + 1, firstVert + j + pData->mAxisSubdivCount, firstVert + pData->mAxisSubdivCount + j + 1);
				}
				ADD_TRI(firstVert + pData->mAxisSubdivCount - 1, firstVert + 2*pData->mAxisSubdivCount - 1, firstVert);
				ADD_TRI(firstVert, firstVert + 2*pData->mAxisSubdivCount - 1, firstVert + pData->mAxisSubdivCount);
			}

			#undef ADD_TRI


			// draw
			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,	// PrimitiveType
				0,						// MinVertexIndex
				vertCount,				// NumVertices
				triCount,				// PrimitiveCount
				inds,					// IndexData
				verts					// VertexStreamZeroData
			);
		}

		VuMatrix	mModelMat;
		VuMatrix	mViewProjMat;
		VuColor		mColor;
		float		mRadius;
		int			mAxisSubdivCount;
		int			mHeightSubdivCount;
		VuVector3	mDirLight;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mViewProjMat = viewProjMat;
	pData->mColor = color;
	pData->mRadius = fRadius;
	pData->mAxisSubdivCount = axisSubdivCount;
	pData->mHeightSubdivCount = heightSubdivCount;
	pData->mDirLight = dirLight;

	if ( color.mA == 255 )
		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, basicShaders()->get3dXyzNorMaterial(), VUNULL, &DrawData::callback);
	else
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_MODULATE_ABOVE_WATER, basicShaders()->get3dXyzNorMaterial(), VUNULL, &DrawData::callback);
}
*/

//*****************************************************************************
void VuGfxUtil::submitClearCommand(VUUINT32 flags, const VuColor &color, float depth, int sequenceNo)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfx::IF()->clear(pData->mFlags, pData->mColor, pData->mDepth);
		}

		VUUINT32	mFlags;
		VuColor		mColor;
		float		mDepth;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));

	pData->mFlags = flags;
	pData->mColor = color;
	pData->mDepth = depth;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::submitSetViewportCommand(const VuRect &rect, int sequenceNo)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfx::IF()->setViewport(pData->mRect);
		}

		VuRect	mRect;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));

	pData->mRect = rect;

	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &DrawData::callback);
}

//*****************************************************************************
void VuGfxUtil::submitSetRenderTargetCommand(const VuSetRenderTargetParams &params, int sequenceNo)
{
	struct CommandData
	{
		static void callback(void *data)
		{
			CommandData *pData = static_cast<CommandData *>(data);

			VuGfx::IF()->setRenderTarget(pData->mParams);
		}
		VuSetRenderTargetParams	mParams;
	};

	CommandData *pData = static_cast<CommandData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(CommandData)));
	pData->mParams = params;
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_BEGIN, sequenceNo, &CommandData::callback);
}

//*****************************************************************************
void VuGfxUtil::clearScreenWithRect(VUUINT32 flags, const VuColor &color, float depth)
{
	VuVertex2dXyz verts[4];
	verts[0].mXyz[0] = 0; verts[0].mXyz[1] = 0; verts[0].mXyz[2] = depth;
	verts[1].mXyz[0] = 0; verts[1].mXyz[1] = 1; verts[1].mXyz[2] = depth;
	verts[2].mXyz[0] = 1; verts[2].mXyz[1] = 0; verts[2].mXyz[2] = depth;
	verts[3].mXyz[0] = 1; verts[3].mXyz[1] = 1; verts[3].mXyz[2] = depth;

	if ( flags & VUGFX_CLEAR_COLOR )
		VuGfx::IF()->setPipelineState(mpClearColorPipelineState);
	else
		VuGfx::IF()->setPipelineState(mpClearNoColorPipelineState);

	if ( flags & VUGFX_CLEAR_DEPTH )
		VuGfx::IF()->setDepthStencilState(mpClearDepthDepthStencilState);
	else
		VuGfx::IF()->setDepthStencilState(mpClearNoDepthDepthStencilState);

	mpBasicShaders->set2dXyzConstants(VuMatrix::identity(), color);

	VuGfx::IF()->beginFullScreenEffect();
	VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 2, verts);
	VuGfx::IF()->endFullScreenEffect();

	// restore default render state
	VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getDefaultDepthStencilState());
}

//*****************************************************************************
const VUUINT16 *VuGfxUtil::getQuadIndexBuffer(int quadCount)
{
	growQuadIndexBuffer(quadCount);

	return &mQuadIndexBuffer.begin();
}

//*****************************************************************************
VuTexture *VuGfxUtil::createWhiteTexture(int width, int height)
{
	VuTextureState state;
	state.mMipFilter = VUGFX_TEXF_NONE;
	VuTexture *pTexture = VuGfx::IF()->createTexture(width, height, 0, VUGFX_FORMAT_A8R8G8B8, state);

	if ( pTexture )
	{
		// set pixels
		VuArray<VUBYTE> data;
		data.resize(width*height*4);
		memset(&data.begin(), 0xff, data.size());

		pTexture->setData(0, &data.begin(), data.size());
	}

	return pTexture;
}

//*****************************************************************************
VuVertexBuffer *VuGfxUtil::createBlackVertexColors()
{
	VUUINT32 black = VuColor(0,0,0);

	VuArray<VUUINT32> colors(0);
	colors.resize(BLACK_VERTEX_COLOR_COUNT);
	for ( VUUINT32 *p = &colors.begin(); p != &colors.end(); p++ )
		*p = black;

	VuVertexBuffer *pVertexBuffer = VuGfx::IF()->createVertexBuffer(4*BLACK_VERTEX_COLOR_COUNT, 0);

	if ( pVertexBuffer )
	{
		pVertexBuffer->setData(&colors.begin(), 4*BLACK_VERTEX_COLOR_COUNT);
	}

	return pVertexBuffer;
}

//*****************************************************************************
void VuGfxUtil::growQuadIndexBuffer(int quadCount)
{
	int oldCount = mQuadIndexBuffer.size()/6;
	int newCount = quadCount;

	if ( newCount > oldCount )
	{
		mQuadIndexBuffer.resize(newCount*6);
		for ( int i = oldCount; i < newCount; i++ )
		{
			VUUINT16 *pIndexData = &mQuadIndexBuffer.begin() + i*6;
			VUUINT16 baseIndex = (VUUINT16)(i*4);

			pIndexData[0] = baseIndex + 0;
			pIndexData[1] = baseIndex + 1;
			pIndexData[2] = baseIndex + 2;
			pIndexData[3] = baseIndex + 0;
			pIndexData[4] = baseIndex + 2;
			pIndexData[5] = baseIndex + 3;
		}
	}
}

//*****************************************************************************
void VuGfxUtil::configLowTextureLOD(bool value)
{
	mLowTextureLOD = value;

	const VuAssetFactory::AssetNames &textures = VuAssetFactory::IF()->getAssetNames("VuTextureAsset");
	for ( VuAssetFactory::AssetNames::const_iterator iter = textures.begin(); iter != textures.end(); iter++ )
	{
		if ( VuAsset *pAsset = VuAssetFactory::IF()->findAsset("VuTextureAsset", iter->c_str()) )
		{
			VuTextureAsset *pTextureAsset = static_cast<VuTextureAsset *>(pAsset);
			if ( pTextureAsset->getScaleLowSpec() )
			{
				VuAssetFactory::IF()->reloadAsset(pAsset);
			}
		}
	}

	const VuAssetFactory::AssetNames &cubeTextures = VuAssetFactory::IF()->getAssetNames("VuCubeTextureAsset");
	for ( VuAssetFactory::AssetNames::const_iterator iter = cubeTextures.begin(); iter != cubeTextures.end(); iter++ )
	{
		if ( VuAsset *pAsset = VuAssetFactory::IF()->findAsset("VuCubeTextureAsset", iter->c_str()) )
		{
			VuCubeTextureAsset *pCubeTextureAsset = static_cast<VuCubeTextureAsset *>(pAsset);
			if ( pCubeTextureAsset->getScaleLowSpec() )
			{
				VuAssetFactory::IF()->reloadAsset(pAsset);
			}
		}
	}
}

//*****************************************************************************
void VuGfxUtil::configLowModelLOD(bool value)
{
	mLowModelLOD = value;
}

//*****************************************************************************
void VuGfxUtil::configUltraModelLOD(bool value)
{
	mUltraModelLOD = value;
}

//*****************************************************************************
void VuGfxUtil::configShaderLOD(int value)
{
	mShaderLOD = value;

	const VuAssetFactory::AssetNames &materials = VuAssetFactory::IF()->getAssetNames("VuMaterialAsset");
	for ( VuAssetFactory::AssetNames::const_iterator iter = materials.begin(); iter != materials.end(); iter++ )
	{
		if ( VuAsset *pAsset = VuAssetFactory::IF()->findAsset("VuMaterialAsset", iter->c_str()) )
		{
			VuMaterialAsset *pMaterialAsset = static_cast<VuMaterialAsset *>(pAsset);
			if ( pMaterialAsset->mHasShaderLODs )
			{
				VuAssetFactory::IF()->reloadAsset(pAsset);
			}
		}
	}
}
