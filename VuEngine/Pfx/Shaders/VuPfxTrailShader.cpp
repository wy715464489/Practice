//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PfxTrailShader class
// 
//*****************************************************************************

#include "VuPfxTrailShader.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/Patterns/VuPfxTrailPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxTrailParticle.h"
#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Dev/VuDevProfile.h"


// static functions
struct PfxTrailShaderDrawData
{
	static void callback(void *data)
	{
		PfxTrailShaderDrawData *pData = static_cast<PfxTrailShaderDrawData *>(data);
		pData->mpShader->draw(pData);
	}
	VuPfxTrailShader		*mpShader;
	const VuPfxTrailPattern	*mpParams;
	VuMatrix				mTransform;
	VuAabb					mAabb;
	int						mCount;
};

struct PfxTrailVert
{
	float		mX, mY, mZ;
	float		mU, mV;
	VUUINT32	mColor;
};
static const int sMaxPfxTrailParticleDrawCount = VuScratchPad::SIZE/(2*sizeof(PfxTrailVert));

struct TrailParticle
{
	VuVector3	mPosition;
	VuVector3	mAxis;
	float		mTexCoord;
	VuColor		mColor;
	float		mExtent;
};


// translucency type lookup
static VuGfxSort::eTranslucencyType sTransLookup[VuPfxTrailPattern::SORTING_COUNT][VuPfxTrailPattern::BLEND_MODE_COUNT] =
{
	{ VuGfxSort::TRANS_ADDITIVE_ABOVE_WATER, VuGfxSort::TRANS_MODULATE_ABOVE_WATER }, // SORT_ABOVE_WATER
	{ VuGfxSort::TRANS_ADDITIVE_BELOW_WATER, VuGfxSort::TRANS_MODULATE_BELOW_WATER }, // SORT_BELOW_WATER
	{ VuGfxSort::TRANS_UI_ADDITIVE, VuGfxSort::TRANS_UI_MODULATE }, // SORT_UI
};
VU_COMPILE_TIME_ASSERT(sizeof(sTransLookup)/sizeof(sTransLookup[0][0]) == VuPfxTrailPattern::SORTING_COUNT*VuPfxTrailPattern::BLEND_MODE_COUNT);


//*****************************************************************************
VuPfxTrailShader::VuPfxTrailShader()
{
}

//*****************************************************************************
VuPfxTrailShader::~VuPfxTrailShader()
{
	for ( int i = 0; i < VuPfxTrailPattern::BLEND_MODE_COUNT; i++ )
		VuGfxSort::IF()->releaseMaterial(mpMaterials[i]);
}

//*****************************************************************************
bool VuPfxTrailShader::load()
{
	const char *strShaderName = "Pfx/Trail";

	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(strShaderName);
	if ( !pShaderAsset )
		return VUERROR("Unable to find shader '%s'.", strShaderName);

	// create vertex declaration
	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2, VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 20, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(24));
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

	VuGfxSortMaterialDesc desc;

	// create additive material
	{
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_ONE;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		mpMaterials[VuPfxTrailPattern::BM_ADDITIVE] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// create modulated material
	{
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_INVSRCALPHA;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		mpMaterials[VuPfxTrailPattern::BM_MODULATE] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// clean up
	pVD->removeRef();
	VuAssetFactory::IF()->releaseAsset(pShaderAsset);

	// get shader constants (all materials use same shader program)
	VuShaderProgram *pSP = mpMaterials[0]->mpShaderProgram;

	mConstants.miColorSampler = pSP->getSamplerIndexByName("gColorTexture");

	if ( mConstants.miColorSampler < 0 )
	{
		return VUERROR("Invalid shader '%s'.", strShaderName);
	}

	return true;
}

//*****************************************************************************
void VuPfxTrailShader::submit(const VuCamera &camera, const VuPfxTrailPatternInstance *pTrailPatternInstance)
{
	int count = pTrailPatternInstance->mParticles.size();
	VUASSERT(count <= sMaxPfxTrailParticleDrawCount, "VuPfxTrailShader::submit() too many particles");

	if ( count >= 2 )
	{
		// get pattern params
		const VuPfxTrailPattern *pParams = static_cast<const VuPfxTrailPattern *>(pTrailPatternInstance->mpParams);

		// determine translucency
		VuGfxSort::eTranslucencyType transType = sTransLookup[pParams->mSorting][pParams->mBlendMode];

		// calculate depth
		float depth = (pTrailPatternInstance->mAabb.getCenter() - camera.getEyePosition()).mag()/camera.getFarPlane();
		depth = VuMin(depth, 1.0f);

		// allocate
		int size = sizeof(PfxTrailShaderDrawData) + sizeof(TrailParticle)*count;
		PfxTrailShaderDrawData *pData = static_cast<PfxTrailShaderDrawData *>(VuGfxSort::IF()->allocateCommandMemory(size));
		TrailParticle *pDst = (TrailParticle *)(pData + 1);

		for ( const VuPfxParticle *pSrc = pTrailPatternInstance->mParticles.front(); pSrc; pSrc = pSrc->next() )
		{
			const VuPfxTrailParticle *pt = static_cast<const VuPfxTrailParticle *>(pSrc);

			pDst->mPosition = pSrc->mPosition;
			pDst->mAxis = pt->mAxis;
			pDst->mTexCoord = pt->mTexCoord;
			pDst->mColor.fromVector4(pSrc->mColor*pTrailPatternInstance->mpSystemInstance->mColor);
			pDst->mExtent = 0.5f*pSrc->mScale;
			pDst++;
		}

		pData->mpShader = this;
		pData->mpParams = pParams;
		pData->mTransform = pTrailPatternInstance->getDrawTransform();
		pData->mAabb = pTrailPatternInstance->mAabb;
		pData->mCount = VuMin(count, sMaxPfxTrailParticleDrawCount);

		VuGfxSortMaterial *pMaterial = mpMaterials[pParams->mBlendMode];
		if ( pParams->mBlendMode == VuPfxTrailPattern::BM_ADDITIVE )
			VuGfxSort::IF()->submitDrawCommand<false>(transType, pMaterial, VUNULL, &PfxTrailShaderDrawData::callback);
		else
			VuGfxSort::IF()->submitDrawCommand<true>(transType, pMaterial, VUNULL, &PfxTrailShaderDrawData::callback, depth);
	}
}

//*****************************************************************************
void VuPfxTrailShader::draw(const PfxTrailShaderDrawData *pDrawData)
{
	VU_PROFILE_GFX("PfxTrail");

	const VuPfxTrailPattern *pParams = pDrawData->mpParams;

	Constants &constants = mConstants;

	const VuCamera &camera = VuGfxSort::IF()->getRenderCamera();

	// set textures
	VuTextureAsset *pTextureAsset = pParams->mpTextureAssetProperty->getAsset();
	VuGfx::IF()->setTexture(constants.miColorSampler, pTextureAsset ? pTextureAsset->getTexture() : VuGfxUtil::IF()->whiteTexture());

	// batch verts using scratch pad
	void *pScratchPad = VuScratchPad::get(VuScratchPad::GRAPHICS);
	PfxTrailVert *pVert = (PfxTrailVert *)pScratchPad;
	const TrailParticle *p = (const TrailParticle *)(pDrawData + 1);

	VuVector3 vCamEye = camera.getEyePosition();

	#define SUBMIT_VERTS(pos, texCoord, axis, col)													\
	{																								\
		pVert->mX = pos.mX + axis.mX; pVert->mY = pos.mY + axis.mY; pVert->mZ = pos.mZ + axis.mZ;	\
		pVert->mU = texCoord; pVert->mV = 0.0f;														\
		pVert->mColor = col;																		\
		pVert++;																					\
		pVert->mX = pos.mX - axis.mX; pVert->mY = pos.mY - axis.mY; pVert->mZ = pos.mZ - axis.mZ;	\
		pVert->mU = texCoord; pVert->mV = 1.0f;														\
		pVert->mColor = col;																		\
		pVert++;																					\
	}

	if ( pParams->mTrailType == VuPfxTrailPattern::TT_2D_TRAIL )
	{
		// first pos
		VuVector3 vPos = pDrawData->mTransform.transform(p->mPosition);
		VuVector3 vNextPos = pDrawData->mTransform.transform((p + 1)->mPosition);
		VuVector3 vAxis = VuCross(vNextPos - vPos, vPos - vCamEye).normal()*p->mExtent;
		SUBMIT_VERTS(vPos, p->mTexCoord, vAxis, p->mColor);
		p++;
		
		VuVector3 vLastPos;
		for ( int i = 1; i < pDrawData->mCount - 1; i++ )
		{
			vLastPos = vPos;
			vPos = vNextPos;
			vNextPos = pDrawData->mTransform.transform((p + 1)->mPosition);
			vAxis = VuCross(vNextPos - vLastPos, vPos - vCamEye).normal()*p->mExtent;
			SUBMIT_VERTS(vPos, p->mTexCoord, vAxis, p->mColor);
			p++;
		}

		// last pos
		vLastPos = vPos;
		vPos = pDrawData->mTransform.transform(p->mPosition);
		vAxis = VuCross(vPos - vLastPos, vPos - vCamEye).normal()*p->mExtent;
		SUBMIT_VERTS(vPos, p->mTexCoord, vAxis, p->mColor);

		// submit batch
		VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, (pDrawData->mCount - 1)*2, pScratchPad);
	}
	else
	{
		for ( int i = 0; i < pDrawData->mCount; i++ )
		{
			VuVector3 vPos = pDrawData->mTransform.transform(p->mPosition);
			VuVector3 vAxis = pDrawData->mTransform.transformNormal(p->mAxis)*p->mExtent;
			SUBMIT_VERTS(vPos, p->mTexCoord, vAxis, p->mColor);
			p++;
		}

		// disable culling
		VuGfx::IF()->setCullMode(VUGFX_CULL_NONE);

		// submit batch
		VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, (pDrawData->mCount - 1)*2, pScratchPad);

		// restore default render state
		VuGfx::IF()->setCullMode(VUGFX_CULL_CW);
	}
}
