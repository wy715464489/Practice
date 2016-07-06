//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the pipeline state interface class.
//
//*****************************************************************************

#include "VuMetalPipelineState.h"
#include "VuMetalShaderProgram.h"
#include "VuMetalVertexDeclaration.h"
#include "VuMetalGfx.h"
#include "VuEngine/Util/VuHash.h"


// static variables

static struct MetalPipelineStateData
{
	typedef std::map<VUUINT64, VuMetalPipelineState *> PipelineStates;
	PipelineStates mPipelineStates;
} sMetalPipelineStateData;


MTLBlendFactor sBlendFactorLookup[] =
{
	MTLBlendFactorZero,                     // VUGFX_BLEND_ZERO,
	MTLBlendFactorOne,                      // VUGFX_BLEND_ONE,
	MTLBlendFactorSourceColor,              // VUGFX_BLEND_SRCCOLOR,
	MTLBlendFactorOneMinusSourceColor,      // VUGFX_BLEND_INVSRCCOLOR,
	MTLBlendFactorSourceAlpha,              // VUGFX_BLEND_SRCALPHA,
	MTLBlendFactorOneMinusSourceAlpha,      // VUGFX_BLEND_INVSRCALPHA,
	MTLBlendFactorDestinationAlpha,         // VUGFX_BLEND_DESTALPHA,
	MTLBlendFactorOneMinusDestinationAlpha, // VUGFX_BLEND_INVDESTALPHA,
	MTLBlendFactorDestinationColor,         // VUGFX_BLEND_DESTCOLOR,
	MTLBlendFactorOneMinusDestinationColor, // VUGFX_BLEND_INVDESTCOLOR,
	MTLBlendFactorSourceAlphaSaturated,     // VUGFX_BLEND_SRCALPHASAT,
};
VU_COMPILE_TIME_ASSERT(sizeof(sBlendFactorLookup)/sizeof(sBlendFactorLookup[0]) == VUGFX_BLEND_MODE_COUNT);


//*****************************************************************************
VuMetalPipelineState::VuMetalPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params):
	VuPipelineState(pSP, pVD, params)
{
}

//*****************************************************************************
VuMetalPipelineState::~VuMetalPipelineState()
{
	sMetalPipelineStateData.mPipelineStates.erase(mHash);
}

//*****************************************************************************
VuMetalPipelineState *VuMetalPipelineState::create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	// find a matchine pipeline state
	VUUINT64 hash = VuHash::fnv64(&params, sizeof(params));
	hash = VuHash::fnv64(&pSP, sizeof(pSP), hash);
	hash = VuHash::fnv64(&pVD, sizeof(pVD), hash);
	
	MetalPipelineStateData::PipelineStates::const_iterator iter = sMetalPipelineStateData.mPipelineStates.find(hash);
	if ( iter != sMetalPipelineStateData.mPipelineStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}
	
	VuMetalShaderProgram *pMetalSP = (VuMetalShaderProgram *)pSP;
	VuMetalVertexDeclaration *pMetalVD = (VuMetalVertexDeclaration *)pVD;
	
	MTLRenderPipelineDescriptor *pDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	pDescriptor.vertexFunction = pMetalSP->mShaders[VuShaderProgram::VERTEX_SHADER].mMtlFunction;
	pDescriptor.fragmentFunction = pMetalSP->mShaders[VuShaderProgram::PIXEL_SHADER].mMtlFunction;
	pDescriptor.vertexDescriptor = pMetalVD->mpMTLVertexDescriptor;
	pDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
	
	if ( !params.mDepthRenderingHint )
	{
		pDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
		pDescriptor.colorAttachments[0].blendingEnabled = params.mAlphaBlendEnabled;
		pDescriptor.colorAttachments[0].writeMask = params.mColorWriteEnabled ? MTLColorWriteMaskAll : MTLColorWriteMaskNone;
		
		if ( params.mAlphaBlendEnabled )
		{
			MTLBlendFactor metalSrcBlendFactor = sBlendFactorLookup[params.mSrcBlendMode];
			MTLBlendFactor metalDstBlendFactor = sBlendFactorLookup[params.mDstBlendMode];
			
			pDescriptor.colorAttachments[0].sourceAlphaBlendFactor = metalSrcBlendFactor;
			pDescriptor.colorAttachments[0].sourceRGBBlendFactor = metalSrcBlendFactor;
			pDescriptor.colorAttachments[0].destinationAlphaBlendFactor = metalDstBlendFactor;
			pDescriptor.colorAttachments[0].destinationRGBBlendFactor = metalDstBlendFactor;
		}
	}
	
	NSError *error;
	id<MTLRenderPipelineState> renderPipelineState = [VuMetalGfx::getDevice() newRenderPipelineStateWithDescriptor:pDescriptor error:&error];
	if ( renderPipelineState == nil )
	{
		NSLog(@"%@",[error localizedDescription]);
		return VUNULL;
	}
	
	// create new pipeline state
	VuMetalPipelineState *pPipelineState = new VuMetalPipelineState(pSP, pVD, params);
	pPipelineState->mHash = hash;
	pPipelineState->mMTLRenderPipelineState = renderPipelineState;
	
	sMetalPipelineStateData.mPipelineStates[hash] = pPipelineState;
	
	return pPipelineState;
}
