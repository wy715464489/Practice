//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Basic shaders.
// 
//*****************************************************************************

#include "VuBasicShaders.h"

#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"



// macros

#define CREATE_SHADER(name) \
{ \
	VuVertexDeclarationParams vdParams; \
	create##name##VD(vdParams); \
	if ( !m##name.create("Basic/" #name, vdParams) ) \
		return VUERROR("Unable to create basic shader '%s'.", #name); \
}


// vertex declarations

//*****************************************************************************
static void create2dXyzVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mStreams.push_back(VuVertexDeclarationStream(12));
};

//*****************************************************************************
static void create2dXyzUvVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2, VUGFX_DECL_USAGE_TEXCOORD, 0));
	params.mStreams.push_back(VuVertexDeclarationStream(20));
};

//*****************************************************************************
static void create2dXyzColVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,    0));
	params.mStreams.push_back(VuVertexDeclarationStream(16));
};

//*****************************************************************************
static void create2dXyzUvMaskVD(VuVertexDeclarationParams &params)
{
	// NOTE: this uses the same vertexdeclaration as the 2dXyzUv
	create2dXyzUvVD(params);
}

//*****************************************************************************
static void create3dXyzVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mStreams.push_back(VuVertexDeclarationStream(12));
};

//*****************************************************************************
static void create3dXyzUvVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2, VUGFX_DECL_USAGE_TEXCOORD, 0));
	params.mStreams.push_back(VuVertexDeclarationStream(20));
};

//*****************************************************************************
static void create3dXyzColVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,    0));
	params.mStreams.push_back(VuVertexDeclarationStream(16));
};

//*****************************************************************************
static void create3dXyzNorVD(VuVertexDeclarationParams &params)
{
	params.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
	params.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_NORMAL,   0));
	params.mStreams.push_back(VuVertexDeclarationStream(24));
};


//*****************************************************************************
bool VuBasicShaders::init()
{
	// create basic shaders
	CREATE_SHADER(2dXyz);
	{
		VuShaderProgram *pSP = m2dXyz.mpShaderProgram;
		mConsts2dXyz.mhColor = pSP->getConstantByName("gColor");
		mConsts2dXyz.mhTransform = pSP->getConstantByName("gTransform");
	}

	CREATE_SHADER(2dXyzUv);
	{
		VuShaderProgram *pSP = m2dXyzUv.mpShaderProgram;
		mConsts2dXyzUv.mhColor = pSP->getConstantByName("gColor");
		mConsts2dXyzUv.mhTransform = pSP->getConstantByName("gTransform");
		mConsts2dXyzUv.miTexture = pSP->getSamplerIndexByName("tex0");
	}

	CREATE_SHADER(2dXyzCol);
	{
		VuShaderProgram *pSP = m2dXyzCol.mpShaderProgram;
		mConsts2dXyzCol.mhTransform = pSP->getConstantByName("gTransform");
	}

	CREATE_SHADER(2dXyzUvMask);
	{
		VuShaderProgram *pSP = m2dXyzUvMask.mpShaderProgram;
		mConsts2dXyzUvMask.mhColor = pSP->getConstantByName("gColor");
		mConsts2dXyzUvMask.mhTransform = pSP->getConstantByName("gTransform");
		mConsts2dXyzUvMask.miTexture = pSP->getSamplerIndexByName("tex0");
		mConsts2dXyzUvMask.miMaskTexture = pSP->getSamplerIndexByName("tex1");
	}

	CREATE_SHADER(3dXyz);
	{
		VuShaderProgram *pSP = m3dXyz.mpShaderProgram;
		mConsts3dXyz.mhColor = pSP->getConstantByName("gColor");
		mConsts3dXyz.mhModelViewProjMatrix = pSP->getConstantByName("gModelViewProjMatrix");
	}

	CREATE_SHADER(3dXyzUv);
	{
		VuShaderProgram *pSP = m3dXyzUv.mpShaderProgram;
		mConsts3dXyzUv.mhColor = pSP->getConstantByName("gColor");
		mConsts3dXyzUv.mhModelViewProjMatrix = pSP->getConstantByName("gModelViewProjMatrix");
		mConsts3dXyzUv.miTexture = pSP->getSamplerIndexByName("tex0");
	}

	CREATE_SHADER(3dXyzCol);
	{
		VuShaderProgram *pSP = m3dXyzCol.mpShaderProgram;
		mConsts3dXyzCol.mhModelViewProjMatrix = pSP->getConstantByName("gModelViewProjMatrix");
	}

	CREATE_SHADER(3dXyzNor);
	{
		VuShaderProgram *pSP = m3dXyzNor.mpShaderProgram;
		mConsts3dXyzNor.mhDirLightWorld = pSP->getConstantByName("gDirLightWorld");
		mConsts3dXyzNor.mhColor = pSP->getConstantByName("gColor");
		mConsts3dXyzNor.mhModelViewProjMatrix = pSP->getConstantByName("gModelViewProjMatrix");
		mConsts3dXyzNor.mhModelMatrix = pSP->getConstantByName("gModelMatrix");
	}

	return true;
}

//*****************************************************************************
void VuBasicShaders::release()
{
	m2dXyz.release();
	m2dXyzUv.release();
	m2dXyzCol.release();
	m2dXyzUvMask.release();
	m3dXyz.release();
	m3dXyzUv.release();
	m3dXyzCol.release();
	m3dXyzNor.release();
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get2dXyzMaterial(eFlavor flavor)
{
	return m2dXyz.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set2dXyzConstants(const VuMatrix &transform, const VuColor &color)
{
	VuShaderProgram *pSP = m2dXyz.mpShaderProgram;

	pSP->setConstantColor4(mConsts2dXyz.mhColor, color);
	pSP->setConstantMatrix(mConsts2dXyz.mhTransform, transform);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get2dXyzUvMaterial(eFlavor flavor)
{
	return m2dXyzUv.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set2dXyzUvConstants(const VuMatrix &transform, const VuColor &color)
{
	VuShaderProgram *pSP = m2dXyzUv.mpShaderProgram;

	pSP->setConstantColor4(mConsts2dXyzUv.mhColor, color);
	pSP->setConstantMatrix(mConsts2dXyzUv.mhTransform, transform);
}

//*****************************************************************************
void VuBasicShaders::set2dXyzUvTexture(VuTexture *pTexture)
{
	VuGfx::IF()->setTexture(mConsts2dXyzUv.miTexture, pTexture);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get2dXyzColMaterial(eFlavor flavor)
{
	return m2dXyzCol.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set2dXyzColConstants(const VuMatrix &transform)
{
	VuShaderProgram *pSP = m2dXyzCol.mpShaderProgram;

	pSP->setConstantMatrix(mConsts2dXyzCol.mhTransform, transform);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get2dXyzUvMaskMaterial(eFlavor flavor)
{
	return m2dXyzUvMask.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set2dXyzUvMaskConstants(const VuMatrix &transform, const VuColor &color)
{
	VuShaderProgram *pSP = m2dXyzUvMask.mpShaderProgram;

	pSP->setConstantColor4(mConsts2dXyzUvMask.mhColor, color);
	pSP->setConstantMatrix(mConsts2dXyzUvMask.mhTransform, transform);
}

//*****************************************************************************
void VuBasicShaders::set2dXyzUvMaskTextures(VuTexture *pTexture, VuTexture *pMaskTexture)
{
	VuGfx::IF()->setTexture(mConsts2dXyzUvMask.miTexture, pTexture);
	VuGfx::IF()->setTexture(mConsts2dXyzUvMask.miMaskTexture, pMaskTexture);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get3dXyzMaterial(eFlavor flavor)
{
	return m3dXyz.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set3dXyzConstants(const VuMatrix &modelViewProjMatrix, const VuColor &color)
{
	VuShaderProgram *pSP = m3dXyz.mpShaderProgram;

	pSP->setConstantColor4(mConsts3dXyz.mhColor, color);
	pSP->setConstantMatrix(mConsts3dXyz.mhModelViewProjMatrix, modelViewProjMatrix);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get3dXyzUvMaterial(eFlavor flavor)
{
	return m3dXyzUv.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set3dXyzUvConstants(const VuMatrix &modelViewProjMatrix, const VuColor &color)
{
	VuShaderProgram *pSP = m3dXyzUv.mpShaderProgram;

	pSP->setConstantColor4(mConsts3dXyzUv.mhColor, color);
	pSP->setConstantMatrix(mConsts3dXyzUv.mhModelViewProjMatrix, modelViewProjMatrix);
}

//*****************************************************************************
void VuBasicShaders::set3dXyzUvTexture(VuTexture *pTexture)
{
	VuGfx::IF()->setTexture(mConsts3dXyzUv.miTexture, pTexture);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get3dXyzColMaterial(eFlavor flavor)
{
	return m3dXyzCol.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set3dXyzColConstants(const VuMatrix &modelViewProjMatrix)
{
	VuShaderProgram *pSP = m3dXyzCol.mpShaderProgram;

	pSP->setConstantMatrix(mConsts3dXyzCol.mhModelViewProjMatrix, modelViewProjMatrix);
}

//*****************************************************************************
VuGfxSortMaterial *VuBasicShaders::get3dXyzNorMaterial(eFlavor flavor)
{
	return m3dXyzNor.mpMaterials[flavor];
}

//*****************************************************************************
void VuBasicShaders::set3dXyzNorConstants(const VuMatrix &modelMatrix, const VuMatrix &viewProjMatrix, const VuVector3 &dirLightWorld, const VuColor &color)
{
	VuShaderProgram *pSP = m3dXyzNor.mpShaderProgram;

	VuMatrix modelViewProjMatrix = modelMatrix*viewProjMatrix;

	pSP->setConstantVector3(mConsts3dXyzNor.mhDirLightWorld, dirLightWorld);
	pSP->setConstantColor4(mConsts3dXyzNor.mhColor, color);
	pSP->setConstantMatrix(mConsts3dXyzNor.mhModelViewProjMatrix, modelViewProjMatrix);
	pSP->setConstantMatrix(mConsts3dXyzNor.mhModelMatrix, modelMatrix);
}

//*****************************************************************************
bool VuBasicShaders::VuBasicShader::create(const char *strShaderAsset, const VuVertexDeclarationParams &params)
{
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(strShaderAsset);
	if ( pShaderAsset == VUNULL )
		return false;

	mpShaderProgram = pShaderAsset->getShaderProgram();
	mpShaderProgram->addRef();

	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(params, mpShaderProgram);
	if ( pVD == VUNULL )
		return false;

	// opaque
	{
		VuPipelineStateParams psParams;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);
		if ( pPS == VUNULL )
			return false;

		VuGfxSortMaterialDesc desc;
		mpMaterials[FLV_OPAQUE] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// modulated
	{
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_INVSRCALPHA;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);
		if ( pPS == VUNULL )
			return false;

		VuGfxSortMaterialDesc desc;
		mpMaterials[FLV_MODULATED] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// additive
	{
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_ONE;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);
		if ( pPS == VUNULL )
			return false;

		VuGfxSortMaterialDesc desc;
		mpMaterials[FLV_ADDITIVE] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	// depth
	{
		VuPipelineStateParams psParams;
		psParams.mColorWriteEnabled = false;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(mpShaderProgram, pVD, psParams);
		if ( pPS == VUNULL )
			return false;

		VuGfxSortMaterialDesc desc;
		mpMaterials[FLV_DEPTH] = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
	}

	VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	pVD->removeRef();

	return true;
}

//*****************************************************************************
void VuBasicShaders::VuBasicShader::release()
{
	mpShaderProgram->removeRef();
	for ( int i = 0; i < NUM_FLAVORS; i++ )
		VuGfxSort::IF()->releaseMaterial(mpMaterials[i]);
}
