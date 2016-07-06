//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PfxQuadShader class
// 
//*****************************************************************************

#include "VuPfxQuadShader.h"
#include "VuEngine/Pfx/VuPfxSystem.h"
#include "VuEngine/Pfx/Patterns/VuPfxQuadPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxQuadParticle.h"
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
#include "VuEngine/Gfx/VuGfxSettings.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Dev/VuDevProfile.h"


// flavors
enum eFlavor { FLAVOR_SIMPLE, FLAVOR_FOG, FLAVOR_TILE, FLAVOR_TILE_FOG, FLAVOR_CLIP, FLAVOR_CLIP_FOG, FLAVOR_CLIP_TILE, FLAVOR_CLIP_TILE_FOG, FLAVOR_COUNT };

class VuQuadShaderFlavor
{
public:
	VuQuadShaderFlavor();
	~VuQuadShaderFlavor();

	bool	load(const char *shaderName, const VuVertexDeclarationParams &vdParams);

	VuGfxSortMaterial		*mpMaterials[VuPfxQuadPattern::BLEND_MODE_COUNT];

	struct Constants
	{
		VUHANDLE	mhSpConstClipThreshold;
		VUINT		miColorSampler;
		VUINT		miTileSampler;
	};
	Constants		mConstants;
};

// static functions
struct PfxQuadShaderDrawData
{
	static void callback(void *data)
	{
		PfxQuadShaderDrawData *pData = static_cast<PfxQuadShaderDrawData *>(data);
		pData->mpShader->draw(pData);
	}
	VuPfxQuadShader			*mpShader;
	eFlavor					mFlavor;
	const VuPfxQuadPattern	*mpParams;
	VuMatrix				mTransform;
	VuAabb					mAabb;
	int						mCount;
	float					mScale;
	VuVector4				mColor;
	int						mBlendMode;
};

struct PfxQuadVert
{
	float		mX, mY, mZ;
	VUUINT32	mColor;
	float		mU, mV, mTileU, mTileV;
};
static const int sMaxPfxQuadParticleDrawCount = VuScratchPad::SIZE/(4*sizeof(PfxQuadVert));


// translucency type lookup
static VuGfxSort::eTranslucencyType sTransLookup[VuPfxQuadPattern::SORTING_COUNT][VuPfxQuadPattern::BLEND_MODE_COUNT] =
{
	{ VuGfxSort::TRANS_ADDITIVE_ABOVE_WATER, VuGfxSort::TRANS_MODULATE_ABOVE_WATER }, // SORT_ABOVE_WATER
	{ VuGfxSort::TRANS_ADDITIVE_BELOW_WATER, VuGfxSort::TRANS_MODULATE_BELOW_WATER }, // SORT_BELOW_WATER
	{ VuGfxSort::TRANS_UI_ADDITIVE, VuGfxSort::TRANS_UI_MODULATE }, // SORT_UI
};
VU_COMPILE_TIME_ASSERT(sizeof(sTransLookup)/sizeof(sTransLookup[0][0]) == VuPfxQuadPattern::SORTING_COUNT*VuPfxQuadPattern::BLEND_MODE_COUNT);


//*****************************************************************************
VuPfxQuadShader::VuPfxQuadShader()
{
	mpFlavors = new VuQuadShaderFlavor[FLAVOR_COUNT];
}

//*****************************************************************************
VuPfxQuadShader::~VuPfxQuadShader()
{
	delete[] mpFlavors;
}

//*****************************************************************************
bool VuPfxQuadShader::load()
{
	// create vertex declaration
	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 16, VUGFX_DECL_TYPE_FLOAT4, VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(32));

	// load flavors
	if ( !mpFlavors[FLAVOR_SIMPLE].load("Pfx/Quad/Simple", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_FOG].load("Pfx/Quad/Fog", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_TILE].load("Pfx/Quad/Tile", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_TILE_FOG].load("Pfx/Quad/TileFog", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_CLIP].load("Pfx/Quad/Clip", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_CLIP_FOG].load("Pfx/Quad/ClipFog", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_CLIP_TILE].load("Pfx/Quad/ClipTile", vdParams) ) return false;
	if ( !mpFlavors[FLAVOR_CLIP_TILE_FOG].load("Pfx/Quad/ClipTileFog", vdParams) ) return false;

	return true;
}

//*****************************************************************************
void VuPfxQuadShader::submit(const VuCamera &camera, const VuPfxQuadPatternInstance *pQuadPatternInstance)
{
	VUASSERT(pQuadPatternInstance->mParticles.size() <= sMaxPfxQuadParticleDrawCount, "VuPfxQuadShader::submit() too many particles");

	// get pattern params
	const VuPfxQuadPattern *pParams = static_cast<const VuPfxQuadPattern *>(pQuadPatternInstance->mpParams);

	// determine translucency
	VuGfxSort::eTranslucencyType transType = sTransLookup[pParams->mSorting][pParams->mBlendMode];

	// calculate depth
	float depth = (pQuadPatternInstance->mAabb.getCenter() - camera.getEyePosition()).mag()/camera.getFarPlane();
	depth = VuMin(depth, 1.0f);

	// precalculate fade params
	const VuVector3 &camEyePos = camera.getEyePosition();
	const VuVector3 &camFwd = camera.getTransform().getAxisY();
	const float invFadeRange = 1.0f/(pParams->mNearFadeMax - pParams->mNearFadeMin);
	const VuMatrix &xform = pQuadPatternInstance->getDrawTransform();

	// allocate worst-case size
	int particlesSize = sizeof(VuPfxQuadParticle)*pQuadPatternInstance->mParticles.size();
	int size = sizeof(PfxQuadShaderDrawData) + particlesSize;

	PfxQuadShaderDrawData *pData = static_cast<PfxQuadShaderDrawData *>(VuGfxSort::IF()->allocateCommandMemory(size));
	VuPfxQuadParticle *pDst = (VuPfxQuadParticle *)(pData + 1);

	int count = 0;
	for ( const VuPfxParticle *pSrc = pQuadPatternInstance->mParticles.front(); pSrc; pSrc = pSrc->next() )
	{
		// handle camera near fade
		VuVector3 pos = xform.transform(pSrc->mPosition);
		float dist = VuDot((pos - camEyePos), camFwd);
		float fade = (dist - pParams->mNearFadeMin)*invFadeRange;
		if ( fade > 0.0f )
		{
			fade = VuMin(fade, 1.0f);
			VU_MEMCPY(pDst, particlesSize, pSrc, sizeof(VuPfxQuadParticle));
			pDst->mColor.mW *= fade;

			count++;
			pDst++;
		}
	}

	if ( count > 0 )
	{
		// shrink allocated size
		size = sizeof(PfxQuadShaderDrawData) + sizeof(VuPfxQuadParticle)*count;
		VuGfxSort::IF()->resizeCommandMemory(pData, size);

		int bClip = pParams->mClipThreshold > 0;
		int bTile = pParams->mpTileTextureAssetProperty->getAsset() != VUNULL;
		int bFog = pParams->mFogEnabled;
		eFlavor flavor = (eFlavor)((bClip<<2) + (bTile<<1) + bFog);

		pData->mpShader = this;
		pData->mFlavor = flavor;
		pData->mpParams = pParams;
		pData->mTransform = xform;
		pData->mAabb = pQuadPatternInstance->mAabb;
		pData->mCount = VuMin(count, sMaxPfxQuadParticleDrawCount);
		pData->mScale = pQuadPatternInstance->mpSystemInstance->mScale;
		pData->mColor = pQuadPatternInstance->mpSystemInstance->mColor;
		pData->mBlendMode = pParams->mBlendMode;

		VuGfxSortMaterial *pMaterial = mpFlavors[flavor].mpMaterials[pParams->mBlendMode];
		if ( pParams->mBlendMode == VuPfxQuadPattern::BM_ADDITIVE )
			VuGfxSort::IF()->submitDrawCommand<false>(transType, pMaterial, VUNULL, &PfxQuadShaderDrawData::callback);
		else
			VuGfxSort::IF()->submitDrawCommand<true>(transType, pMaterial, VUNULL, &PfxQuadShaderDrawData::callback, depth);
	}
	else
	{
		// nevermind
		VuGfxSort::IF()->resizeCommandMemory(pData, 0);
	}
}

//*****************************************************************************
void VuPfxQuadShader::draw(const PfxQuadShaderDrawData *pDrawData)
{
	VU_PROFILE_GFX("PfxQuad");

	const VuPfxQuadPattern *pParams = pDrawData->mpParams;
	VuQuadShaderFlavor *pFlavor = &mpFlavors[pDrawData->mFlavor];

	VuGfxSortMaterial *pMaterial = pFlavor->mpMaterials[pDrawData->mBlendMode];
	VuQuadShaderFlavor::Constants &constants = pFlavor->mConstants;

	VuShaderProgram *pSP = pMaterial->mpShaderProgram;

	const VuCamera &camera = VuGfxSort::IF()->getRenderCamera();
	//const VuGfxSettings &gfxSettings = VuGfxSort::IF()->getRenderGfxSettings();

	// constants
	if ( constants.mhSpConstClipThreshold )
		pSP->setConstantFloat(constants.mhSpConstClipThreshold, pParams->mClipThreshold);

	// set textures
	VuTextureAsset *pTextureAsset = pParams->mpTextureAssetProperty->getAsset();
	VuGfx::IF()->setTexture(constants.miColorSampler, pTextureAsset ? pTextureAsset->getTexture() : VUNULL);

	// tile texture
	if ( constants.miTileSampler >= 0 )
	{
		VuGfx::IF()->setTexture(constants.miTileSampler, pParams->mpTileTextureAssetProperty->getAsset()->getTexture());
	}

	VuVector3 camX = camera.getTransform().getAxisX();
	VuVector3 camY = camera.getTransform().getAxisY();
	VuVector3 camZ = camera.getTransform().getAxisZ();

	if (VuAbs(camZ.mZ) > 0.001f)
	{
		camZ.mX = 0.0f;
		camZ.mY = 0.0f;
		camZ.mZ /= VuAbs(camZ.mZ);
		camX = VuCross(camY, camZ);
		camX.normalize();
		camZ = VuCross(camX, camY);
	}

	VuVector3 viewX = camX;
	VuVector3 viewY = camZ;
	VuVector3 viewZ = -camY;

	float offsets[4][2] = { { -1, -1 }, { +1, -1 }, { +1, +1 }, { -1, +1 } };

	// batch verts using scratch pad
	void *pScratchPad = VuScratchPad::get(VuScratchPad::GRAPHICS);
	PfxQuadVert *pVert = (PfxQuadVert *)pScratchPad;
	const VuPfxQuadParticle *p = (const VuPfxQuadParticle *)(pDrawData + 1);
	for ( int i = 0; i < pDrawData->mCount; i++ )
	{
		// rotate screen vectors
		float s, c;
		VuSinCos(p->mRotation, s, c);
		VuVector3 screenX = c*viewX + s*viewY;
		VuVector3 screenY = c*viewY - s*viewX;

		// calculate world velocity
		VuVector3 velWorld = pDrawData->mTransform.transformNormal(p->mLinearVelocity);
		velWorld -= viewZ*VuDot(velWorld, viewZ);
		VuVector3 velWorldNormal = velWorld;
		if ( velWorldNormal.magSquared() > FLT_EPSILON )
			velWorldNormal.normalize();

		// calculate world position
		VuVector3 posWorld = pDrawData->mTransform.transform(p->mPosition);

		// color
		VuColor color;
		color.fromFloat4(pDrawData->mColor.mX*p->mColor.mX, pDrawData->mColor.mY*p->mColor.mY, pDrawData->mColor.mZ*p->mColor.mZ, VuMin(pDrawData->mColor.mW*p->mColor.mW, 1.0f));

		// scale
		float scale = pDrawData->mScale*p->mScale;

		// tile offset
		float tileTime = VuModf(p->mAge, pParams->mTileScrollLoopTime);
		float tileOffsetU = p->mTileOffsetU + tileTime*pParams->mTileScrollSpeedU;
		float tileOffsetV = p->mTileOffsetV + tileTime*pParams->mTileScrollSpeedV;

		// verts
		for ( int j = 0; j < 4; j++ )
		{
			// calculate offset
			VuVector3 offsetWorld = (scale*(offsets[j][0] + pParams->mCenterOffset.mX))*screenX + (scale*(offsets[j][1] + pParams->mCenterOffset.mY))*screenY;

			// apply directional stretch
			float stretchAmount = p->mDirStretch*VuDot(velWorld, offsetWorld);
			stretchAmount = VuClamp(stretchAmount, -pParams->mMaxStretch, pParams->mMaxStretch);
			offsetWorld += stretchAmount*velWorldNormal;

			// apply world scale z
			offsetWorld.mZ *= p->mWorldScaleZ;

			pVert[j].mX = posWorld.mX + offsetWorld.mX;
			pVert[j].mY = posWorld.mY + offsetWorld.mY;
			pVert[j].mZ = posWorld.mZ + offsetWorld.mZ;

			pVert[j].mColor = color;
			pVert[j].mU = 0.5f + 0.5f*offsets[j][0];
			pVert[j].mV = 0.5f - 0.5f*offsets[j][1];
			pVert[j].mTileU = (pVert[j].mU + tileOffsetU)*pParams->mTileScale;
			pVert[j].mTileV = (pVert[j].mV + tileOffsetV)*pParams->mTileScale;
		}

		pVert += 4;
		p++;
	}

	// submit batch
	const VUUINT16 *pIndexData = VuGfxUtil::IF()->getQuadIndexBuffer(pDrawData->mCount);

	VuGfx::IF()->drawIndexedPrimitiveUP(
		VUGFX_PT_TRIANGLELIST,	// PrimitiveType
		0,						// MinVertexIndex
		pDrawData->mCount*4,	// NumVertices
		pDrawData->mCount*2,	// PrimitiveCount
		pIndexData,				// IndexData
		pScratchPad				// VertexStreamZeroData
	);
}

//*****************************************************************************
VuQuadShaderFlavor::VuQuadShaderFlavor()
{
}

//*****************************************************************************
VuQuadShaderFlavor::~VuQuadShaderFlavor()
{
	for ( int i = 0; i < VuPfxQuadPattern::BLEND_MODE_COUNT; i++ )
		VuGfxSort::IF()->releaseMaterial(mpMaterials[i]);
}

//*****************************************************************************
bool VuQuadShaderFlavor::load(const char *shaderName, const VuVertexDeclarationParams &vdParams)
{
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderName);

	// create vertex declaration
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

	VuGfxSortMaterialDesc desc;

	// create additive material
	{
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_ONE;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		mpMaterials[VuPfxQuadPattern::BM_ADDITIVE] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// create modulated material
	{
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_INVSRCALPHA;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		mpMaterials[VuPfxQuadPattern::BM_MODULATE] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// clean up
	pVD->removeRef();
	VuAssetFactory::IF()->releaseAsset(pShaderAsset);

	// get shader constants (all materials use same shader program)
	VuShaderProgram *pSP = mpMaterials[0]->mpShaderProgram;

	mConstants.mhSpConstClipThreshold = pSP->getConstantByName("gClipThreshold");
	mConstants.miTileSampler = pSP->getSamplerIndexByName("gTileTexture");
	mConstants.miColorSampler = pSP->getSamplerIndexByName("gColorTexture");

	if ( mConstants.miColorSampler < 0 )
	{
		return VUERROR("Invalid shader '%s'.", shaderName);
	}

	return true;
}
