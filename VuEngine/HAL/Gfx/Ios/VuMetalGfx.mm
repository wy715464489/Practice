//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Ios Gfx Metal HAL.
//
//*****************************************************************************

#import <UIKit/UIKit.h>

#include "VuMetalGfx.h"
#include "VuMetalRenderTarget.h"
#include "VuMetalDepthRenderTarget.h"
#include "VuMetalShadowRenderTarget.h"
#include "VuMetalVertexBuffer.h"
#include "VuMetalIndexBuffer.h"
#include "VuMetalVertexDeclaration.h"
#include "VuMetalTexture.h"
#include "VuMetalCubeTexture.h"
#include "VuMetalShaderProgram.h"
#include "VuMetalPipelineState.h"
#include "VuMetalDepthStencilState.h"
#include "VuMetalSamplerState.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Dev/VuDevStat.h"


static id <MTLDevice> sMetalDevice = nil;
static CAMetalLayer *sMetalLayer = nil;
static float sScaleFactor = 1.0f;


MTLCullMode sCullModeLookup[] =
{
	MTLCullModeNone,  // VUGFX_CULL_NONE
	MTLCullModeBack,  // VUGFX_CULL_CW
	MTLCullModeFront, // VUGFX_CULL_CCW
};
VU_COMPILE_TIME_ASSERT(sizeof(sCullModeLookup)/sizeof(sCullModeLookup[0]) == VUGFX_CULL_MODE_COUNT);

MTLPrimitiveType sPrimitiveTypeLookup [] =
{
	MTLPrimitiveTypePoint,         // VUGFX_PT_POINTLIST,
	MTLPrimitiveTypeLine,          // VUGFX_PT_LINELIST,
	MTLPrimitiveTypeLineStrip,     // VUGFX_PT_LINESTRIP,
	MTLPrimitiveTypeTriangle,      // VUGFX_PT_TRIANGLELIST,
	MTLPrimitiveTypeTriangleStrip, // VUGFX_PT_TRIANGLESTRIP,
};
VU_COMPILE_TIME_ASSERT(sizeof(sPrimitiveTypeLookup)/sizeof(sPrimitiveTypeLookup[0]) == VUGFX_PRIMITIVE_TYPE_COUNT);

MTLLoadAction sLoadActionLookup[] =
{
	MTLLoadActionDontCare, // LoadActionDontCare
	MTLLoadActionLoad,	   // LoadActionLoad
	MTLLoadActionClear,    // LoadActionClear
};


//*****************************************************************************
VuMetalGfx::VuMetalGfx():
	mDisplayWidth(0),
	mDisplayHeight(0),
	mCurRenderTargetWidth(0),
	mCurRenderTargetHeight(0),
	mCurViewport(0,0,1,1),
	mpCurIndexBuffer(VUNULL),
	mpCurShaderProgram(VUNULL),
	mCurVertexStride(0),
	mDynamicBufferIndex(0),
	mDynamicBufferOffset(0),
	mCurDynamicBufferSize(0),
	mMaxDynamicBufferSize(0)
{
}

//*****************************************************************************
bool VuMetalGfx::init(VUHANDLE hWindow, VUHANDLE hDevice)
{
	sMetalLayer.device = sMetalDevice;
	sMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	sMetalLayer.framebufferOnly = TRUE;
	
	mDisplayWidth = VuRound(sMetalLayer.frame.size.width*sScaleFactor);
	mDisplayHeight = VuRound(sMetalLayer.frame.size.height*sScaleFactor);
	
	mInflightSemaphore = dispatch_semaphore_create(IN_FLIGHT_COMMAND_BUFFER_COUNT);
	
	mCommandQueue = [sMetalDevice newCommandQueue];
	
	// create depth texture
	{
		MTLTextureDescriptor *desc = [MTLTextureDescriptor
									  texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
									  width:mDisplayWidth
									  height:mDisplayHeight
									  mipmapped:NO];
		mDepthTexture = [sMetalDevice newTextureWithDescriptor:desc];
	}
	
	mCurRenderTargetWidth = mDisplayWidth;
	mCurRenderTargetHeight = mDisplayHeight;
	
	// create dynamic buffers
	for ( int i = 0; i < IN_FLIGHT_COMMAND_BUFFER_COUNT; i++ )
		mDynamicBuffers[i] = [sMetalDevice newBufferWithLength:DYNAMIC_BUFFER_SIZE options:0];
	
	return true;
}

//*****************************************************************************
void VuMetalGfx::release()
{
	VU_SAFE_RELEASE(mpCurIndexBuffer);
	
	for ( int i = 0; i < IN_FLIGHT_COMMAND_BUFFER_COUNT; i++ )
		mDynamicBuffers[i] = nil;
	
	mDepthTexture = nil;
	mCommandQueue = nil;
	sMetalLayer = nil;
	sMetalDevice = nil;
}

//*****************************************************************************
void VuMetalGfx::resize(VUHANDLE hDisplay, int width, int height)
{
	VUASSERT(0, "Not expected to reach this!");
}

//*****************************************************************************
void VuMetalGfx::getDisplaySize(VUHANDLE hDisplay, int &width, int &height)
{
	width = mDisplayWidth;
	height = mDisplayHeight;
}

//*****************************************************************************
VuRenderTarget *VuMetalGfx::createRenderTarget(int width, int height)
{
	return VuMetalRenderTarget::create(width, height);
}

//*****************************************************************************
VuDepthRenderTarget *VuMetalGfx::createDepthRenderTarget(int width, int height)
{
	return VuMetalDepthRenderTarget::create(width, height);
}

//*****************************************************************************
VuShadowRenderTarget *VuMetalGfx::createShadowRenderTarget(int width, int height, int count)
{
	return VuMetalShadowRenderTarget::create(width, height, count);
}

//*****************************************************************************
VuFxRenderTarget *VuMetalGfx::createFxRenderTarget(int width, int height, VuGfxFormat format)
{
	VUASSERT(0, "Not Implemented!");
	return VUNULL;
}

//*****************************************************************************
VuVertexBuffer *VuMetalGfx::createVertexBuffer(int size, VUUINT32 usageFlags)
{
	return VuMetalVertexBuffer::create(size, usageFlags);
}

//*****************************************************************************
VuIndexBuffer *VuMetalGfx::createIndexBuffer(int count, VUUINT32 usageFlags)
{
	return VuMetalIndexBuffer::create(count, usageFlags);
}

//*****************************************************************************
VuVertexDeclaration *VuMetalGfx::createVertexDeclaration(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	return VuMetalVertexDeclaration::create(params, pShaderProgram);
}

//*****************************************************************************
VuTexture *VuMetalGfx::createTexture(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	return VuMetalTexture::create(width, height, usageFlags, format, state);
}

//*****************************************************************************
VuTexture *VuMetalGfx::loadTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuMetalTexture::load(reader, skipLevels);
}

//*****************************************************************************
VuCubeTexture *VuMetalGfx::loadCubeTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuMetalCubeTexture::load(reader, skipLevels);
}

//*****************************************************************************
VuShaderProgram *VuMetalGfx::loadShaderProgram(VuBinaryDataReader &reader)
{
	return VuMetalShaderProgram::load(reader);
}

//*****************************************************************************
VuPipelineState *VuMetalGfx::createPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	return VuMetalPipelineState::create(pSP, pVD, params);
}

//*****************************************************************************
VuDepthStencilState *VuMetalGfx::createDepthStencilState(const VuDepthStencilStateParams &params)
{
	return VuMetalDepthStencilState::create(params);
}

//*****************************************************************************
void VuMetalGfx::getCurRenderTargetSize(int &width, int &height)
{
	width = mCurRenderTargetWidth;
	height = mCurRenderTargetHeight;
}

//*****************************************************************************
void VuMetalGfx::setRenderTarget(const VuSetRenderTargetParams &params)
{
	if ( mEncoder != nil )
	{
		[mEncoder endEncoding];
		mEncoder = nil;
	}
	
	MTLClearColor clearColor;
	clearColor.red = params.mClearColor.mR;
	clearColor.green = params.mClearColor.mG;
	clearColor.blue = params.mClearColor.mB;
	clearColor.alpha = params.mClearColor.mA;
	
	double clearDepth = params.mClearDepth;
	
	// set up render pass descriptor for frame buffer
	MTLRenderPassDescriptor	*pRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	
	pRenderPassDescriptor.colorAttachments[0].loadAction = sLoadActionLookup[params.mColorLoadAction];
	pRenderPassDescriptor.colorAttachments[0].clearColor = clearColor;
	pRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	pRenderPassDescriptor.depthAttachment.loadAction = sLoadActionLookup[params.mDepthLoadAction];
	pRenderPassDescriptor.depthAttachment.clearDepth = clearDepth;
	pRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
	
	if ( params.mpRenderTarget )
	{
		VuMetalRenderTarget *pMetalRenderTarget = (VuMetalRenderTarget *)params.mpRenderTarget;
		
		pRenderPassDescriptor.colorAttachments[0].texture = pMetalRenderTarget->mpColorTexture->mMTLTexture;
		pRenderPassDescriptor.depthAttachment.texture = pMetalRenderTarget->mDepthTexture;

		mCurRenderTargetWidth = pMetalRenderTarget->getWidth();
		mCurRenderTargetHeight = pMetalRenderTarget->getHeight();
	}
	else
	{
		pRenderPassDescriptor.colorAttachments[0].texture = mDrawable.texture;
		pRenderPassDescriptor.depthAttachment.texture = mDepthTexture;
		
		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	mEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:pRenderPassDescriptor];
	
	// set defaults
	[mEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
	[mEncoder setCullMode:MTLCullModeBack];
	VuMetalDepthStencilState *pDepthStencilState = (VuMetalDepthStencilState *)VuGfxUtil::IF()->getDefaultDepthStencilState();
	[mEncoder setDepthStencilState:pDepthStencilState->mMTLDepthStencilState];
	
	mCurViewport = VuRect(0,0,1,1);
}

//*****************************************************************************
void VuMetalGfx::setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget)
{
	VUASSERT(0, "Not Implemented!");
}

//*****************************************************************************
void VuMetalGfx::setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	if ( mEncoder != nil )
	{
		[mEncoder endEncoding];
		mEncoder = nil;
	}
	
	VuMetalShadowRenderTarget *pMetalShadowRenderTarget = (VuMetalShadowRenderTarget *)pShadowRenderTarget;
	
	// set up render pass descriptor for frame buffer
	MTLRenderPassDescriptor	*pRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	
	pRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	pRenderPassDescriptor.depthAttachment.clearDepth = 1.0;
	pRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	pRenderPassDescriptor.depthAttachment.texture = pMetalShadowRenderTarget->mMTLTexture;
	pRenderPassDescriptor.depthAttachment.slice = layer;
	
	mEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:pRenderPassDescriptor];
	
	mCurRenderTargetWidth = pShadowRenderTarget->getWidth();
	mCurRenderTargetHeight = pShadowRenderTarget->getHeight();
	
	// set defaults
	[mEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
	[mEncoder setCullMode:MTLCullModeBack];
	VuMetalDepthStencilState *pDepthStencilState = (VuMetalDepthStencilState *)VuGfxUtil::IF()->getDefaultDepthStencilState();
	[mEncoder setDepthStencilState:pDepthStencilState->mMTLDepthStencilState];
	
	mCurViewport = VuRect(0,0,1,1);
}

//*****************************************************************************
void VuMetalGfx::setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget)
{
	VUASSERT(0, "Not Implemented!");
}

//*****************************************************************************
bool VuMetalGfx::beginScene(VUHANDLE hDisplay)
{
	if ( !VuGfx::beginScene(hDisplay) )
		return false;

	return true;
}

//*****************************************************************************
bool VuMetalGfx::endScene(VUHANDLE hDisplay)
{
	if ( !VuGfx::endScene(hDisplay) )
		return false;
	
	if ( mEncoder != nil )
	{
		[mEncoder endEncoding];
		mEncoder = nil;
	}
	
	mCurDynamicBufferSize = mDynamicBufferOffset;
	
	mDynamicBufferIndex = (mDynamicBufferIndex + 1)%IN_FLIGHT_COMMAND_BUFFER_COUNT;
	mDynamicBufferOffset = 0;

	return true;
}

//*****************************************************************************
bool VuMetalGfx::clear(VUUINT32 flags, const VuColor &color, float depth)
{
	VuGfxUtil::IF()->clearScreenWithRect(flags, color, depth);
	
	return true;
}

//*****************************************************************************
bool VuMetalGfx::setViewport(const VuRect &rect)
{
	if ( mCurViewport != rect )
	{
		VuRect screenRect = rect*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);
		
		MTLViewport vp;
		vp.originX = VuRound(screenRect.mX);
		vp.originY = VuRound(screenRect.mY);
		vp.width = VuRound(screenRect.mWidth);
		vp.height = VuRound(screenRect.mHeight);
		vp.znear = 0.0;
		vp.zfar = 1.0;
		
		[mEncoder setViewport:vp];
		
		mCurViewport = rect;
	}
	
	return true;
}

//*****************************************************************************
bool VuMetalGfx::setScissorRect(const VuRect *pRect)
{
	VuRect rect(0,0,1,1);

	if ( pRect )
		rect = *pRect;

	VuRect screenRect = rect*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);
	
	MTLScissorRect scissorRect;
	scissorRect.x = VuRound(screenRect.mX);
	scissorRect.y = VuRound(screenRect.mY);
	scissorRect.width = VuRound(screenRect.mWidth);
	scissorRect.height = VuRound(screenRect.mHeight);
	
	[mEncoder setScissorRect:scissorRect];
	
	return true;
}

//*****************************************************************************
bool VuMetalGfx::setVertexBuffer(VuVertexBuffer *pVertexBuffer)
{
	VuMetalVertexBuffer *pMetalVertexBuffer = (VuMetalVertexBuffer *)pVertexBuffer;
	
	[mEncoder setVertexBuffer:pMetalVertexBuffer->mMTLBuffer offset:0 atIndex:0];

	return true;
}

//*****************************************************************************
bool VuMetalGfx::setIndexBuffer(VuIndexBuffer *pIndexBuffer)
{
	if ( pIndexBuffer != mpCurIndexBuffer )
	{
		VU_SAFE_RELEASE(mpCurIndexBuffer);
		
		mpCurIndexBuffer = (VuMetalIndexBuffer *)pIndexBuffer;
		mpCurIndexBuffer->addRef();
	}

	return true;
}

//*****************************************************************************
void VuMetalGfx::setPipelineState(VuPipelineState *pPipelineState)
{
	VuMetalPipelineState *pMetalPipelineState = (VuMetalPipelineState *)pPipelineState;
	
	[mEncoder setRenderPipelineState:pMetalPipelineState->mMTLRenderPipelineState];

	mpCurShaderProgram = (VuMetalShaderProgram *)pMetalPipelineState->mpShaderProgram;
	mpCurShaderProgram->mShaders[VuShaderProgram::VERTEX_SHADER].mConstantBufferDirtyBits = 0xffffffff;
	mpCurShaderProgram->mShaders[VuShaderProgram::PIXEL_SHADER].mConstantBufferDirtyBits = 0xffffffff;
	
	mCurVertexStride = pMetalPipelineState->mpVertexDeclaration->mParams.mStreams[0].mStride;
}

//*****************************************************************************
void VuMetalGfx::setDepthStencilState(VuDepthStencilState *pDepthStencilState)
{
	VuMetalDepthStencilState *pMetalDepthStencilState = (VuMetalDepthStencilState *)pDepthStencilState;

	[mEncoder setDepthStencilState:pMetalDepthStencilState->mMTLDepthStencilState];
}

//*****************************************************************************
void VuMetalGfx::setCullMode(VuGfxCullMode cullMode)
{
	MTLCullMode metalCullMode = sCullModeLookup[cullMode];
	[mEncoder setCullMode:metalCullMode];
}

//*****************************************************************************
bool VuMetalGfx::setTexture(int sampler, VuBaseTexture *pBaseTexture)
{
	if ( pBaseTexture )
	{
		if ( pBaseTexture->isDerivedFrom(VuTexture::msRTTI) )
		{
			VuMetalTexture *pMetalTexture = (VuMetalTexture *)pBaseTexture;
			[mEncoder setFragmentTexture:pMetalTexture->mMTLTexture atIndex:sampler];
			[mEncoder setFragmentSamplerState:pMetalTexture->mpSamplerState->mMTLSamplerState atIndex:sampler];
		}
		else if ( pBaseTexture->isDerivedFrom(VuCubeTexture::msRTTI) )
		{
			VuMetalCubeTexture *pMetalCubeTexture = (VuMetalCubeTexture *)pBaseTexture;
			[mEncoder setFragmentTexture:pMetalCubeTexture->mMTLCubeTexture atIndex:sampler];
			[mEncoder setFragmentSamplerState:pMetalCubeTexture->mpSamplerState->mMTLSamplerState atIndex:sampler];
		}
	}
	else
	{
		[mEncoder setFragmentTexture:nil atIndex:sampler];
	}

	return true;
}

//*****************************************************************************
bool VuMetalGfx::setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	if ( pShadowRenderTarget )
	{
		VuMetalShadowRenderTarget *pMetalShadowRenderTarget = (VuMetalShadowRenderTarget *)pShadowRenderTarget;
		[mEncoder setFragmentTexture:pMetalShadowRenderTarget->mMTLTexture atIndex:sampler];
	}
	else
	{
		[mEncoder setFragmentTexture:nil atIndex:sampler];
	}
	
	return true;
}

//*****************************************************************************
void VuMetalGfx::drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount)
{
	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	
	MTLPrimitiveType metalPrimitiveType = sPrimitiveTypeLookup[primitiveType];
	
	[mEncoder drawPrimitives:metalPrimitiveType
				 vertexStart:startVertex
				 vertexCount:vertexCount];
	
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuMetalGfx::drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount)
{
	updateState();
	
	int vertexCount = calcVertexCount(primitiveType, primitiveCount);

	MTLPrimitiveType metalPrimitiveType = sPrimitiveTypeLookup[primitiveType];
	
	[mEncoder drawIndexedPrimitives:metalPrimitiveType
						 indexCount:vertexCount
						  indexType:MTLIndexTypeUInt16
						indexBuffer:mpCurIndexBuffer->mMTLBuffer
				  indexBufferOffset:startIndex*2];
	
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuMetalGfx::drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer)
{
	VuMetalVertexBuffer *pMetalVertexColorBuffer = (VuMetalVertexBuffer *)pVertexColorBuffer;
	[mEncoder setVertexBuffer:pMetalVertexColorBuffer->mMTLBuffer offset:0 atIndex:1];
	
	drawIndexedPrimitive(primitiveType, minIndex, numVerts, startIndex, primitiveCount);
}

//*****************************************************************************
void VuMetalGfx::drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData)
{
	updateState();
	
	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	
	MTLPrimitiveType metalPrimitiveType = sPrimitiveTypeLookup[primitiveType];

	int vertexDataSize = vertexCount*mCurVertexStride;
	if ( mDynamicBufferOffset + vertexDataSize > DYNAMIC_BUFFER_SIZE )
	{
		VUPRINTF("Dynamic Buffer overflow!");
		return;
	}
	
	VUBYTE *pBuffer = (VUBYTE *)[mDynamicBuffers[mDynamicBufferIndex] contents];
	
	memcpy(&pBuffer[mDynamicBufferOffset], pVertexData, vertexDataSize);
	[mEncoder setVertexBuffer:mDynamicBuffers[mDynamicBufferIndex] offset:mDynamicBufferOffset atIndex:0];
	mDynamicBufferOffset += vertexDataSize;
			  
	[mEncoder drawPrimitives:metalPrimitiveType
				 vertexStart:0
				 vertexCount:vertexCount];
	
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuMetalGfx::drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData)
{
	updateState();
	
	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	
	MTLPrimitiveType metalPrimitiveType = sPrimitiveTypeLookup[primitiveType];
	
	int vertexDataSize = numVerts*mCurVertexStride;
	int indexDataSize = vertexCount*2;
	if ( mDynamicBufferOffset + vertexDataSize + indexDataSize > DYNAMIC_BUFFER_SIZE )
	{
		VUPRINTF("Dynamic Buffer overflow!");
		return;
	}

	VUBYTE *pBuffer = (VUBYTE *)[mDynamicBuffers[mDynamicBufferIndex] contents];
	
	// copy vertex data
	memcpy(&pBuffer[mDynamicBufferOffset], pVertexData, vertexDataSize);
	[mEncoder setVertexBuffer:mDynamicBuffers[mDynamicBufferIndex] offset:mDynamicBufferOffset atIndex:0];
	mDynamicBufferOffset += vertexDataSize;

	// copy index data
	memcpy(&pBuffer[mDynamicBufferOffset], pIndexData, indexDataSize);
	int indexBufferOffset = mDynamicBufferOffset;
	mDynamicBufferOffset += vertexDataSize;
	
	[mEncoder drawIndexedPrimitives:metalPrimitiveType
						 indexCount:vertexCount
						  indexType:MTLIndexTypeUInt16
						indexBuffer:mDynamicBuffers[mDynamicBufferIndex]
				  indexBufferOffset:indexBufferOffset];
	
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuMetalGfx::syncPreDraw()
{
	dispatch_semaphore_wait(mInflightSemaphore, DISPATCH_TIME_FOREVER);
	
	mDrawable = [sMetalLayer nextDrawable];
	mCommandBuffer = [mCommandQueue commandBuffer];
}

//*****************************************************************************
void VuMetalGfx::syncPostDraw()
{
	[mCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer>) {
		dispatch_semaphore_signal(mInflightSemaphore);
	}];
	[mCommandBuffer presentDrawable:mDrawable];
	[mCommandBuffer commit];
	
	mCommandBuffer = nil;
	mDrawable = nil;
}

//*****************************************************************************
void VuMetalGfx::resetStats()
{
	VuGfx::resetStats();
	
	mMaxDynamicBufferSize = VuMax(mMaxDynamicBufferSize, mCurDynamicBufferSize);
	mCurDynamicBufferSize = 0;
}

//*****************************************************************************
void VuMetalGfx::printStats()
{
	VuGfx::printStats();
	
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "Gfx" )
			{
				pPage->printf("DB: size-%dK max-%dK cur-%dK\n", DYNAMIC_BUFFER_SIZE/1024, mMaxDynamicBufferSize/1024, mCurDynamicBufferSize/1024);
			}
		}
	}
}

//*****************************************************************************
bool VuMetalGfx::checkForMetal()
{
	static bool sMetalCheckComplete = false;
	
	if ( !sMetalCheckComplete )
	{
		// check if the device is running iOS 8.0 or later
		NSString *reqSysVer = @"8.0";
		NSString *curSysVer = [[UIDevice currentDevice] systemVersion];
		BOOL osVersionSupported = ([curSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
		
		if ( osVersionSupported )
		{
			sMetalDevice = MTLCreateSystemDefaultDevice();
		}
		
		sMetalCheckComplete = true;
	}
	
	return sMetalDevice != nil;
}

//*****************************************************************************
id <MTLDevice> VuMetalGfx::getDevice()
{
	return sMetalDevice;
}

//*****************************************************************************
void VuMetalGfx::setMetalLayer(CAMetalLayer *pMetalLayer, float scaleFactor)
{
	sMetalLayer = pMetalLayer;
	sScaleFactor = scaleFactor;
}

//*****************************************************************************
int VuMetalGfx::calcVertexCount(VuGfxPrimitiveType primitiveType, int primitiveCount)
{
	static int sLookup[][2] =
	{
		{ 1, 0 },	// VUGFX_PT_POINTLIST		vc = pc
		{ 2, 0 },	// VUGFX_PT_LINELIST		vc = pc*2
		{ 1, 1 },	// VUGFX_PT_LINESTRIP		vc = pc + 1
		{ 3, 0 },	// VUGFX_PT_TRIANGLELIST	vc = pc*3
		{ 1, 2 },	// VUGFX_PT_TRIANGLESTRIP	vc = pc + 2
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_PRIMITIVE_TYPE_COUNT);
	
	const int *pEntry = sLookup[primitiveType];
	return primitiveCount*pEntry[0] + pEntry[1];
}

//*****************************************************************************
void VuMetalGfx::updateState()
{
	// 16-byte alignment for constants
	mDynamicBufferOffset = VuAlign(mDynamicBufferOffset, 16);
	
	VUBYTE *pBuffer = (VUBYTE *)[mDynamicBuffers[mDynamicBufferIndex] contents];

	// set constant buffers
	if ( mpCurShaderProgram )
	{
		VuMetalShaderProgram::Shader &vertexShader = mpCurShaderProgram->mShaders[VuShaderProgram::VERTEX_SHADER];
		for ( int i = 0; i < vertexShader.mConstantBufferCount; i++ )
		{
			if ( vertexShader.mConstantBufferDirtyBits & (1<<i) )
			{
				VuMetalShaderProgram::ConstantBuffer &constantBuffer = vertexShader.mConstantBuffers[i];
				
				if ( mDynamicBufferOffset + constantBuffer.mSize > DYNAMIC_BUFFER_SIZE )
				{
					VUPRINTF("Dynamic Buffer overflow!");
					return;
				}
				memcpy(&pBuffer[mDynamicBufferOffset], constantBuffer.mpData, constantBuffer.mSize);
				[mEncoder setVertexBuffer:mDynamicBuffers[mDynamicBufferIndex] offset:mDynamicBufferOffset atIndex:constantBuffer.mIndex];
				mDynamicBufferOffset += constantBuffer.mSize;
			}
		}
		vertexShader.mConstantBufferDirtyBits = 0;
	}
	
	VuMetalShaderProgram::Shader &fragmentShader = mpCurShaderProgram->mShaders[VuShaderProgram::PIXEL_SHADER];
	for ( int i = 0; i < fragmentShader.mConstantBufferCount; i++ )
	{
		if ( fragmentShader.mConstantBufferDirtyBits & (1<<i) )
		{
			VuMetalShaderProgram::ConstantBuffer &constantBuffer = fragmentShader.mConstantBuffers[i];
			
			if ( mDynamicBufferOffset + constantBuffer.mSize > DYNAMIC_BUFFER_SIZE )
			{
				VUPRINTF("Dynamic Buffer overflow!");
				return;
			}
			memcpy(&pBuffer[mDynamicBufferOffset], constantBuffer.mpData, constantBuffer.mSize);
			[mEncoder setFragmentBuffer:mDynamicBuffers[mDynamicBufferIndex] offset:mDynamicBufferOffset atIndex:constantBuffer.mIndex];
			mDynamicBufferOffset += constantBuffer.mSize;
		}
	}
	fragmentShader.mConstantBufferDirtyBits = 0;
}
