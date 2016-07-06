//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuBlobShadowShader
// 
//*****************************************************************************

#include "VuBlobShadowShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"


//*****************************************************************************
VuBlobShadowShader::VuBlobShadowShader():
	mpPipelineState(VUNULL)
{
}


//*****************************************************************************
bool VuBlobShadowShader::init()
{
	if ( VuAssetFactory::IF()->doesAssetExist<VuCompiledShaderAsset>("BlobShadow") )
	{
		VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>("BlobShadow");

		// create vertex declaration
		VuVertexDeclarationParams vdParams;
		vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3,  VUGFX_DECL_USAGE_POSITION, 0));
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
		vdParams.mElements.push_back(VuVertexDeclarationElement(0, 20, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,    0));
		vdParams.mStreams.push_back(VuVertexDeclarationStream(24));
		VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

		// create pipeline state
		VuPipelineStateParams psParams;
		psParams.mAlphaBlendEnabled = true;
		psParams.mSrcBlendMode = VUGFX_BLEND_SRCALPHA;
		psParams.mDstBlendMode = VUGFX_BLEND_INVSRCALPHA;
		mpPipelineState = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

		pVD->removeRef();
		VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	}

	return true;
}

//*****************************************************************************
void VuBlobShadowShader::release()
{
	if ( mpPipelineState )
		mpPipelineState->removeRef();
}
