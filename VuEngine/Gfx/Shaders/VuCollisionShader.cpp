//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Collision Shader class
// 
//*****************************************************************************

#include "VuCollisionShader.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"


//*****************************************************************************
VuCollisionShader::VuCollisionShader():
	mpMaterial(VUNULL)
{
}

//*****************************************************************************
bool VuCollisionShader::init()
{
#ifndef VURETAIL
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("Collision");
	if ( pShaderAsset )
	{
		VuVertexDeclarationParams vdParams;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3, VUGFX_DECL_USAGE_POSITION, 0));
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,   0));
		vdParams.mStreams.push_back(VuVertexDeclarationStream(16));
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		VuPipelineStateParams psParams;
		VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		VuGfxSortMaterialDesc desc;
		mpMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);

		pPS->removeRef();
		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);

		VuShaderProgram *pSP = mpMaterial->mpShaderProgram;
		mCollisionModelMatrixHandle = pSP->getConstantByName("gModelMatrix");
		mCollisionDiffuseColorHandle = pSP->getConstantByName("DiffuseColor");
	}
#endif

	return true;
}

//*****************************************************************************
void VuCollisionShader::release()
{
#ifndef VURETAIL
	VuGfxSort::IF()->releaseMaterial(mpMaterial);
#endif
}

//*****************************************************************************
VuGfxSortMaterial *VuCollisionShader::getMaterial()
{
	return mpMaterial;
}

//*****************************************************************************
void VuCollisionShader::setConstants(const VuMatrix &modelMatrix, const VuColor &color)
{
	VuShaderProgram *pSP = mpMaterial->mpShaderProgram;

	pSP->setConstantMatrix(mCollisionModelMatrixHandle, modelMatrix);
	pSP->setConstantColor4(mCollisionDiffuseColorHandle, color);
}
