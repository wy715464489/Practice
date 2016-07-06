//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 interface class for Gfx.
//
//*****************************************************************************

#include <sys/dmem.h>
#include <kernel.h>
#include <video_out.h>

#include "toolkit/toolkit.h"

#include "VuPs4Gfx.h"
#include "VuPs4GfxTypes.h"
#include "VuPs4Texture.h"
#include "VuPs4CubeTexture.h"
#include "VuPs4RenderTarget.h"
#include "VuPs4DepthRenderTarget.h"
#include "VuPs4ShadowRenderTarget.h"
#include "VuPs4FxRenderTarget.h"
#include "VuPs4VertexBuffer.h"
#include "VuPs4IndexBuffer.h"
#include "VuPs4ShaderProgram.h"
#include "VuPs4VertexDeclaration.h"
#include "VuPs4PipelineState.h"
#include "VuPs4DepthStencilState.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Dev/VuDevStat.h"


using namespace sce;

#define PS4_DISPLAY_WIDTH 1920
#define PS4_DISPLAY_HEIGHT 1080
#define PS4_DISPLAY_BUFFER_COUNT 2
#define PS4_NUM_CUE_RING_ENTRIES 64
#define PS4_DRAW_COMMAND_BUFFER_SIZE (4*1024*1024)
#define PS4_CONSTANT_COMMAND_BUFFER_SIZE (2*1024*1024)
#define PS4_DYNAMIC_VB_SIZE (2*1024*1024)
#define PS4_COLOR_FORMAT Gnm::kDataFormatB8G8R8A8Unorm
#define PS4_DEPTH_FORMAT Gnm::kZFormat32Float
#define PS4_STENCIL_FORMAT Gnm::kStencilInvalid
#define PS4_HTILE_ENABLED true
#define PS4_VIDEO_FLIP_RATE 0 // 60 hz

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGfx, VuPs4Gfx);


//*****************************************************************************
VuPs4Gfx::VuPs4Gfx():
	mpCurPipelineState(VUNULL),
	mpCurShaderProgram(VUNULL),
	mpCurVertexDeclaration(VUNULL),
	mpCurIndexBuffer(VUNULL),
	mpCurGnmColorRenderTarget(VUNULL),
	mpCurGnmDepthRenderTarget(VUNULL),
	mpGnmPendingResolveRenderTarget(VUNULL),
	mCurDVBSize(0),
	mMaxDVBSize(0),
	mCurDCBSize(0),
	mMaxDCBSize(0),
	mCurCCBSize(0),
	mMaxCCBSize(0)
{
}

//*****************************************************************************
VuPs4Gfx::~VuPs4Gfx()
{
}

//*****************************************************************************
bool VuPs4Gfx::init(VUHANDLE hWindow, VUHANDLE hDevice)
{
	// allocate CPU memory
	{
		size_t size = VuEngine::IF()->options().mGfxCpuMemorySize;
		int alignment = 2 * 1024 * 1024;
		off_t physicalOffset;
		void *address = NULL;

		int ret = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, size, alignment, SCE_KERNEL_WB_ONION, &physicalOffset);
		if ( ret == SCE_OK )
			ret = sceKernelMapDirectMemory(&address, size, SCE_KERNEL_PROT_CPU_READ|SCE_KERNEL_PROT_CPU_WRITE|SCE_KERNEL_PROT_GPU_ALL, 0, physicalOffset, alignment);

		if ( ret != SCE_OK )
		{
			VUERROR("Unable to allocate Gfx CPU memory");
			return false;
		}

		mOnionMemoryManager.init(address, size);
	}

	// allocate GPU memory
	{
		size_t size = VuEngine::IF()->options().mGfxGpuMemorySize;
		int alignment = 2 * 1024 * 1024;
		off_t physicalOffset;
		void *address = NULL;

		int ret = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, size, alignment, SCE_KERNEL_WC_GARLIC, &physicalOffset);
		if ( ret == SCE_OK )
			ret = sceKernelMapDirectMemory(&address, size, SCE_KERNEL_PROT_CPU_READ|SCE_KERNEL_PROT_CPU_WRITE|SCE_KERNEL_PROT_GPU_ALL, 0, physicalOffset, alignment);

		if ( ret != SCE_OK )
		{
			VUERROR("Unable to allocate Gfx GPU memory");
			return false;
		}

		mGarlicMemoryManager.init(address, size);
	}

	// initialize CPU/GPU memory allocators
	mOnionMemoryAllocator.m_allocate = allocateOnionMemory;
	mOnionMemoryAllocator.m_release = releaseOnionMemory;

	mGarlicMemoryAllocator.m_allocate = allocateGarlicMemory;
	mGarlicMemoryAllocator.m_release = releaseGarlicMemory;

	// initialize toolkit library
	{
		Gnmx::Toolkit::MemoryRequests toolkitMemRequests;
		toolkitMemRequests.initialize();
		Gnmx::Toolkit::addToMemoryRequests(&toolkitMemRequests);

		void *toolkitMemGarlic = garlicAllocator().allocate(toolkitMemRequests.m_garlic.m_sizeAlign);
		void *toolkitMemOnion = onionAllocator().allocate(toolkitMemRequests.m_onion.m_sizeAlign);

		toolkitMemRequests.m_garlic.fulfill(toolkitMemGarlic);
		toolkitMemRequests.m_onion.fulfill(toolkitMemOnion);
		Gnmx::Toolkit::initializeWithMemoryRequests(&toolkitMemRequests);
	}

	// create display buffers
	mpSurfaceAddresses = new void *[PS4_DISPLAY_BUFFER_COUNT];
	mpDisplayBuffers = new DisplayBuffer[PS4_DISPLAY_BUFFER_COUNT];
	for ( int iBuffer = 0; iBuffer < PS4_DISPLAY_BUFFER_COUNT; iBuffer++ )
	{
		DisplayBuffer &displayBuffer = mpDisplayBuffers[iBuffer];

		// create graphics context
		{
			displayBuffer.mCueCpRamShadowBuffer = malloc(Gnmx::ConstantUpdateEngine::computeCpRamShadowSize());
			displayBuffer.mCueHeapAddr = garlicAllocator().allocate(Gnmx::ConstantUpdateEngine::computeHeapSize(PS4_NUM_CUE_RING_ENTRIES), Gnm::kAlignmentOfBufferInBytes);

			displayBuffer.mDrawCommandBuffer = onionAllocator().allocate(PS4_DRAW_COMMAND_BUFFER_SIZE, Gnm::kAlignmentOfBufferInBytes);
			displayBuffer.mConstantCommandBuffer = onionAllocator().allocate(PS4_CONSTANT_COMMAND_BUFFER_SIZE, Gnm::kAlignmentOfBufferInBytes);

			displayBuffer.mContext.init(displayBuffer.mCueCpRamShadowBuffer, displayBuffer.mCueHeapAddr, PS4_NUM_CUE_RING_ENTRIES,
				displayBuffer.mDrawCommandBuffer, PS4_DRAW_COMMAND_BUFFER_SIZE,
				displayBuffer.mConstantCommandBuffer, PS4_CONSTANT_COMMAND_BUFFER_SIZE);
		}

		// create render target
		{
			Gnm::TileMode tileMode;
			GpuAddress::computeSurfaceTileMode(&tileMode, GpuAddress::kSurfaceTypeColorTargetDisplayable, PS4_COLOR_FORMAT, 1);

			Gnm::SizeAlign sizeAlign = displayBuffer.mRenderTarget.init(
				PS4_DISPLAY_WIDTH,
				PS4_DISPLAY_HEIGHT,
				1,
				PS4_COLOR_FORMAT,
				tileMode,
				Gnm::kNumSamples1,
				Gnm::kNumFragments1,
				NULL,
				NULL);
			mpSurfaceAddresses[iBuffer] = garlicAllocator().allocate(sizeAlign);
			displayBuffer.mRenderTarget.setAddresses(mpSurfaceAddresses[iBuffer], 0, 0);
		}

		// create depth stencil surface
		{
			Gnm::DataFormat depthFormat = Gnm::DataFormat::build(PS4_DEPTH_FORMAT);
			Gnm::TileMode depthTileMode;
			GpuAddress::computeSurfaceTileMode(&depthTileMode, GpuAddress::kSurfaceTypeDepthOnlyTarget, depthFormat, 1);

			Gnm::SizeAlign stencilSizeAlign;
			Gnm::SizeAlign htileSizeAlign;
			Gnm::SizeAlign depthTargetSizeAlign = displayBuffer.mDepthTarget.init(
				PS4_DISPLAY_WIDTH,
				PS4_DISPLAY_HEIGHT,
				depthFormat.getZFormat(),
				PS4_STENCIL_FORMAT,
				depthTileMode,
				Gnm::kNumFragments1,
				PS4_STENCIL_FORMAT != Gnm::kStencilInvalid ? &stencilSizeAlign : NULL,
				PS4_HTILE_ENABLED ? &htileSizeAlign : NULL);

			if ( PS4_HTILE_ENABLED )
				displayBuffer.mDepthTarget.setHtileAddress(garlicAllocator().allocate(htileSizeAlign));

			void *stencilMemory = NULL;
			if ( PS4_STENCIL_FORMAT != Gnm::kStencilInvalid )
				stencilMemory = garlicAllocator().allocate(stencilSizeAlign);

			displayBuffer.mDepthTarget.setAddresses(garlicAllocator().allocate(depthTargetSizeAlign), stencilMemory);
		}

		// create dynamic vertex buffer
		displayBuffer.mpDynamicVB = garlicAllocator().allocate(PS4_DYNAMIC_VB_SIZE, Gnm::kAlignmentOfBufferInBytes);
	}

	// initialize video output
	{
		mVideoOutHandle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);

		SceVideoOutBufferAttribute videoOutBufferAttribute;
		sceVideoOutSetBufferAttribute(
			&videoOutBufferAttribute,
			SCE_VIDEO_OUT_PIXEL_FORMAT_A8R8G8B8_SRGB,
			SCE_VIDEO_OUT_TILING_MODE_TILE,
			SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
			mpDisplayBuffers[0].mRenderTarget.getWidth(),
			mpDisplayBuffers[0].mRenderTarget.getHeight(),
			mpDisplayBuffers[0].mRenderTarget.getPitch());

		sceVideoOutRegisterBuffers(mVideoOutHandle, 0, mpSurfaceAddresses, PS4_DISPLAY_BUFFER_COUNT, &videoOutBufferAttribute);

		sceVideoOutSetFlipRate(mVideoOutHandle, PS4_VIDEO_FLIP_RATE);
	}

	// initialize end-of-pipe event queue
	{
		sceKernelCreateEqueue(&mEopEventQueue, "EOP QUEUE");
		Gnm::addEqEvent(mEopEventQueue, Gnm::kEqEventGfxEop, NULL);
	}

	mDisplayWidth = PS4_DISPLAY_WIDTH;
	mDisplayHeight = PS4_DISPLAY_HEIGHT;

	mCurRenderTargetWidth = PS4_DISPLAY_WIDTH;
	mCurRenderTargetHeight = PS4_DISPLAY_HEIGHT;

	mpCurDisplayBuffer = mpDisplayBuffers;
	mCurDisplayBufferIndex = 0;
	mFrameIndex = 0;

	updateState();

	return true;
}

//*****************************************************************************
void VuPs4Gfx::release()
{
	VUASSERT(0, "This is not expected!");
}

//*****************************************************************************
void VuPs4Gfx::resize(VUHANDLE hDisplay, int width, int height)
{
	VUASSERT(0, "This is not expected!");
}

//*****************************************************************************
void VuPs4Gfx::getDisplaySize(VUHANDLE hDisplay, int &width, int &height)
{
	width = mDisplayWidth;
	height = mDisplayHeight;
}

//*****************************************************************************
VuRenderTarget *VuPs4Gfx::createRenderTarget(int width, int height)
{
	return VuPs4RenderTarget::create(width, height);
}

//*****************************************************************************
VuDepthRenderTarget *VuPs4Gfx::createDepthRenderTarget(int width, int height)
{
	return VuPs4DepthRenderTarget::create(width, height);
}

//*****************************************************************************
VuShadowRenderTarget *VuPs4Gfx::createShadowRenderTarget(int width, int height, int count)
{
	return VuPs4ShadowRenderTarget::create(width, height, count);
}

//*****************************************************************************
VuFxRenderTarget *VuPs4Gfx::createFxRenderTarget(int width, int height, VuGfxFormat format)
{
	return VuPs4FxRenderTarget::create(width, height, format);
}

//*****************************************************************************
VuVertexBuffer *VuPs4Gfx::createVertexBuffer(int size, VUUINT32 usageFlags)
{
	return VuPs4VertexBuffer::create(size, usageFlags);
}

//*****************************************************************************
VuIndexBuffer *VuPs4Gfx::createIndexBuffer(int count, VUUINT32 usageFlags)
{
	return VuPs4IndexBuffer::create(count, usageFlags);
}

//*****************************************************************************
VuVertexDeclaration *VuPs4Gfx::createVertexDeclaration(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	return VuPs4VertexDeclaration::create(params, pShaderProgram);
}

//*****************************************************************************
VuTexture *VuPs4Gfx::createTexture(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	return VuPs4Texture::create(width, height, usageFlags, format, state);
}

//*****************************************************************************
VuTexture *VuPs4Gfx::loadTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuPs4Texture::load(reader, skipLevels);
}

//*****************************************************************************
VuCubeTexture *VuPs4Gfx::loadCubeTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuPs4CubeTexture::load(reader, skipLevels);
}

//*****************************************************************************
VuShaderProgram *VuPs4Gfx::loadShaderProgram(VuBinaryDataReader &reader)
{
	return VuPs4ShaderProgram::load(reader);
}

//*****************************************************************************
VuPipelineState *VuPs4Gfx::createPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	return VuPs4PipelineState::create(pSP, pVD, params);
}

//*****************************************************************************
VuDepthStencilState *VuPs4Gfx::createDepthStencilState(const VuDepthStencilStateParams &params)
{
	return VuPs4DepthStencilState::create(params);
}

//*****************************************************************************
void VuPs4Gfx::getCurRenderTargetSize(int &width, int &height)
{
	width = mCurRenderTargetWidth;
	height = mCurRenderTargetHeight;
}

//*****************************************************************************
void VuPs4Gfx::setRenderTarget(const VuSetRenderTargetParams &params)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	VuPs4RenderTarget *pPs4RenderTarget = (VuPs4RenderTarget *)params.mpRenderTarget;

	doPendingResolve();

	if ( pPs4RenderTarget )
	{
		mpCurGnmColorRenderTarget = &pPs4RenderTarget->mColorTarget;
		mpCurGnmDepthRenderTarget = &pPs4RenderTarget->mDepthTarget;

		mCurRenderTargetWidth = pPs4RenderTarget->getWidth();
		mCurRenderTargetHeight = pPs4RenderTarget->getHeight();

		mpGnmPendingResolveRenderTarget = &pPs4RenderTarget->mColorTarget;
	}
	else
	{
		mpCurGnmColorRenderTarget = &mpCurDisplayBuffer->mRenderTarget;
		mpCurGnmDepthRenderTarget = &mpCurDisplayBuffer->mDepthTarget;

		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	gfxc.setRenderTarget(0, mpCurGnmColorRenderTarget);
	gfxc.setDepthRenderTarget(mpCurGnmDepthRenderTarget);

	setViewport(VuRect(0,0,1,1));

	// handle clear
	if ( mpCurGnmColorRenderTarget && (params.mColorLoadAction == VuSetRenderTargetParams::LoadActionClear) )
	{
		Vector4 vColor;
		params.mClearColor.toFloat4(vColor[0], vColor[1], vColor[2], vColor[3]);
		Gnmx::Toolkit::SurfaceUtil::clearRenderTarget(gfxc, mpCurGnmColorRenderTarget, vColor);
	}

	if ( mpCurGnmDepthRenderTarget && (params.mDepthLoadAction == VuSetRenderTargetParams::LoadActionClear) )
	{
		Gnmx::Toolkit::SurfaceUtil::clearDepthTarget(gfxc, mpCurGnmDepthRenderTarget, params.mClearDepth);
	}
}

//*****************************************************************************
void VuPs4Gfx::setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	doPendingResolve();

	mpCurGnmColorRenderTarget = VUNULL;
	mpCurGnmDepthRenderTarget = VUNULL;

	VuPs4DepthRenderTarget *pPs4DepthRenderTarget = (VuPs4DepthRenderTarget *)pDepthRenderTarget;

	mpGnmPendingResolveRenderTarget = &pPs4DepthRenderTarget->mColorTarget;

	gfxc.setRenderTarget(0, &pPs4DepthRenderTarget->mColorTarget);
	gfxc.setDepthRenderTarget(&pPs4DepthRenderTarget->mDepthTarget);

	mCurRenderTargetWidth = pPs4DepthRenderTarget->getWidth();
	mCurRenderTargetHeight = pPs4DepthRenderTarget->getHeight();

	setViewport(VuRect(0,0,1,1));

	// clear
	Gnmx::Toolkit::SurfaceUtil::clearRenderTarget(gfxc, &pPs4DepthRenderTarget->mColorTarget, Vector4(1,1,1,1));
	Gnmx::Toolkit::SurfaceUtil::clearDepthTarget(gfxc, &pPs4DepthRenderTarget->mDepthTarget, 1.0f);
}

//*****************************************************************************
void VuPs4Gfx::setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	doPendingResolve();

	mpCurGnmColorRenderTarget = VUNULL;
	mpCurGnmDepthRenderTarget = VUNULL;

	VuPs4ShadowRenderTarget *pPs4ShadowRenderTarget = (VuPs4ShadowRenderTarget *)pShadowRenderTarget;

	pPs4ShadowRenderTarget->mDepthTarget.setArrayView(layer, layer);
	gfxc.setDepthRenderTarget(&pPs4ShadowRenderTarget->mDepthTarget);
	gfxc.setRenderTarget(0, NULL);

	mCurRenderTargetWidth = pPs4ShadowRenderTarget->getWidth();
	mCurRenderTargetHeight = pPs4ShadowRenderTarget->getHeight();

	setViewport(VuRect(0,0,1,1));

	// clear
	Gnmx::Toolkit::SurfaceUtil::clearDepthTarget(gfxc, &pPs4ShadowRenderTarget->mDepthTarget, 1.0f);
}

//*****************************************************************************
void VuPs4Gfx::setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	doPendingResolve();

	mpCurGnmColorRenderTarget = VUNULL;
	mpCurGnmDepthRenderTarget = VUNULL;

	VuPs4FxRenderTarget *pPs4FxRenderTarget = (VuPs4FxRenderTarget *)pFxRenderTarget;

	mpGnmPendingResolveRenderTarget = &pPs4FxRenderTarget->mColorTarget;

	gfxc.setRenderTarget(0, &pPs4FxRenderTarget->mColorTarget);
	gfxc.setDepthRenderTarget(NULL);

	mCurRenderTargetWidth = pPs4FxRenderTarget->getWidth();
	mCurRenderTargetHeight = pPs4FxRenderTarget->getHeight();

	setViewport(VuRect(0,0,1,1));
}

//*****************************************************************************
bool VuPs4Gfx::beginScene(VUHANDLE hDisplay)
{
	mpCurDisplayBuffer = mpDisplayBuffers + mCurDisplayBufferIndex;
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	// Wait for the EOP event, discarding the first #DisplayBuffers frames.
	// This is important as it ensures that the command buffers are not
	// overridden by the CPU while the GPU is still reading them.
	if ( mFrameIndex >= PS4_DISPLAY_BUFFER_COUNT )
	{
		SceKernelEvent eopEvent;
		int count;
		sceKernelWaitEqueue(mEopEventQueue, &eopEvent, 1, &count, NULL);

		// Safety check: ensure that the GPU passed the prepareFlip
		// command for the DCB that will be used in this frame.
		SceVideoOutFlipStatus flipStatus;
		for(;;)
		{
			// gcQueueNum contains the number of queued flips for which the
			// GPU still needs to execute the relative prepareFlip command.
			sceVideoOutGetFlipStatus(mVideoOutHandle, &flipStatus);
			if( flipStatus.gcQueueNum < PS4_DISPLAY_BUFFER_COUNT )
				break;

			sceKernelUsleep(1);
		}
	}

	// clear out recycle bins
	for ( auto &iter : mOnionRecycleBin )
		mOnionMemoryManager.deallocate(iter);
	mOnionRecycleBin.clear();

	for ( auto &iter : mGarlicRecycleBin )
		mGarlicMemoryManager.deallocate(iter);
	mGarlicRecycleBin.clear();

	// Reset the graphical context and initialize the hardware state.
	gfxc.reset();
	gfxc.initializeDefaultHardwareState();

	// The waitUntilSafeForRendering stalls the GPU until the scan-out
	// operations on the current display buffer have been completed.
	// This command is non-blocking for the CPU.
	gfxc.waitUntilSafeForRendering(mVideoOutHandle, mCurDisplayBufferIndex);

	// Set up the viewport to match the entire screen.
	gfxc.setupScreenViewport(0, 0, mpCurDisplayBuffer->mRenderTarget.getWidth(), mpCurDisplayBuffer->mRenderTarget.getHeight(), 1.0f, 0.0f);

	// Bind the render & depth targets to the context.
	gfxc.setRenderTarget(0, &mpCurDisplayBuffer->mRenderTarget);
	gfxc.setDepthRenderTarget(&mpCurDisplayBuffer->mDepthTarget);

	// default index size
	gfxc.setIndexSize(Gnm::kIndexSize16);

	// standard vertex/pixel shader usage
	gfxc.setActiveShaderStages(Gnm::kActiveShaderStagesVsPs);

	mCurViewport = VuRect(0,0,1,1);

	mpCurPipelineState = VUNULL;
	mpCurIndexBuffer = VUNULL;

	// reset dynamic vertex buffer to beginning
	mpCurDisplayBuffer->mDynamicVBOffset = 0;

	return true;
}

//*****************************************************************************
bool VuPs4Gfx::endScene(VUHANDLE hDisplay)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;
	
	// stats
	mCurDVBSize = mpCurDisplayBuffer->mDynamicVBOffset;
	mCurDCBSize = gfxc.m_dcb.getSizeInBytes();
	mCurCCBSize = gfxc.m_ccb.getSizeInBytes();

	// Trigger a software interrupt to signal the EOP event queue when this
	// command reaches the end of the pipe and the CB/DB caches are flushed.
	gfxc.triggerEndOfPipeInterrupt(Gnm::kEopFlushCbDbCaches, Gnm::kCacheActionNone);

	// Submit the command buffers and request a flip of the display buffer.
	gfxc.submitAndFlip(mVideoOutHandle, mCurDisplayBufferIndex, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);

	// Update the display chain pointers.
	mCurDisplayBufferIndex = (mCurDisplayBufferIndex + 1) % PS4_DISPLAY_BUFFER_COUNT;
	mFrameIndex++;

	// Signal the system that every draw for this frame has been submitted.
	// This function gives permission to the OS to hibernate when all the
	// currently running GPU tasks (graphics and compute) are done.
	Gnm::submitDone();

	mpCurDisplayBuffer = VUNULL; // disallow context use between frames

	return true;
}

//*****************************************************************************
bool VuPs4Gfx::clear(VUUINT32 flags, const VuColor &color, float depth)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	if ( mpCurGnmColorRenderTarget && (flags & VUGFX_CLEAR_COLOR) )
	{
		Vector4 vColor;
		color.toFloat4(vColor[0], vColor[1], vColor[2], vColor[3]);
		Gnmx::Toolkit::SurfaceUtil::clearRenderTarget(gfxc, mpCurGnmColorRenderTarget, vColor);
	}

	if ( mpCurGnmDepthRenderTarget && (flags & VUGFX_CLEAR_DEPTH) )
	{
		Gnmx::Toolkit::SurfaceUtil::clearDepthTarget(gfxc, mpCurGnmDepthRenderTarget, depth);
	}

	return true;
}

//*****************************************************************************
bool VuPs4Gfx::setViewport(const VuRect &rect)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	VuRect screenRect = rect*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);

	uint32_t left = VuRound(screenRect.getLeft());
	uint32_t top = VuRound(screenRect.getTop());
	uint32_t right = VuRound(screenRect.getRight());
	uint32_t bottom = VuRound(screenRect.getBottom());
	gfxc.setupScreenViewport(left, top, right, bottom, 1.0f, 0.0f);

	mCurViewport = rect;

	return true;
}

//*****************************************************************************
bool VuPs4Gfx::setScissorRect(const VuRect *pRect)
{
	VuRect rect(0,0,1,1);

	if ( pRect )
		rect = *pRect;

	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	VuRect screenRect = rect*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);
	gfxc.setGenericScissor(VuRound(screenRect.getLeft()), VuRound(screenRect.getTop()), VuRound(screenRect.getRight()), VuRound(screenRect.getBottom()), sce::Gnm::kWindowOffsetDisable);

	return true;
}

//*****************************************************************************
void VuPs4Gfx::setPipelineState(VuPipelineState *pPipelineState)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	mpCurPipelineState = static_cast<VuPs4PipelineState *>(pPipelineState);

	mpCurShaderProgram = (VuPs4ShaderProgram *)mpCurPipelineState->mpShaderProgram;
	mpCurVertexDeclaration = (VuPs4VertexDeclaration *)mpCurPipelineState->mpVertexDeclaration;

	// set shaders
	VuPs4VertexShader *pPs4VertexShader = static_cast<VuPs4VertexShader *>(mpCurShaderProgram->mapShaders[VuShaderProgram::VERTEX_SHADER]);
	VuPs4PixelShader *pPs4PixelShader = static_cast<VuPs4PixelShader *>(mpCurShaderProgram->mapShaders[VuShaderProgram::PIXEL_SHADER]);

	gfxc.setVsShader(pPs4VertexShader->mpPs4VertexShader, mpCurVertexDeclaration->mShaderModifier, mpCurVertexDeclaration->mpFetchShader);
	gfxc.setPsShader(pPs4PixelShader->mpPs4PixelShader);

	pPs4VertexShader->mConstantBufferDirtyBits = 0xffffffff;
	pPs4PixelShader->mConstantBufferDirtyBits = 0xffffffff;

	// blend control
	gfxc.setBlendControl(0, mpCurPipelineState->mBlendControl);
	gfxc.setRenderTargetMask(mpCurPipelineState->mRenderTargetMask);

	mCurVertexStride = pPipelineState->mpVertexDeclaration->mParams.mStreams[0].mStride;
}

//*****************************************************************************
bool VuPs4Gfx::setVertexBuffer(VuVertexBuffer *pVertexBuffer)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	VuPs4VertexBuffer *pPs4VertexBuffer = static_cast<VuPs4VertexBuffer *>(pVertexBuffer);
	VUBYTE *pVerts = (VUBYTE *)pPs4VertexBuffer->mpBuffer;
	int vertexCount = pPs4VertexBuffer->mSize/mCurVertexStride;

	// set up vertex buffers
	const VuPs4VertexDeclaration::Stream &stream = mpCurVertexDeclaration->mStreams[0];
	Gnm::Buffer vertexBuffers[Gnm::kSlotCountVertexBuffer];
	for ( int i = 0; i < stream.mElementCount; i++ )
		vertexBuffers[i].initAsVertexBuffer(pVerts + stream.mElements[i].mOffset, stream.mElements[i].mDataFormat, stream.mStride, vertexCount);

	gfxc.setVertexBuffers(Gnm::kShaderStageVs, 0, stream.mElementCount, vertexBuffers);

	return true;
}

//*****************************************************************************
bool VuPs4Gfx::setIndexBuffer(VuIndexBuffer *pIndexBuffer)
{
	mpCurIndexBuffer = static_cast<VuPs4IndexBuffer *>(pIndexBuffer);

	return true;
}

//*****************************************************************************
void VuPs4Gfx::setDepthStencilState(VuDepthStencilState *pDepthStencilState)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	VuPs4DepthStencilState *pPs4DepthStencilState = (VuPs4DepthStencilState *)pDepthStencilState;
	gfxc.setDepthStencilControl(pPs4DepthStencilState->mGnmDSC);
}

//*****************************************************************************
void VuPs4Gfx::setCullMode(VuGfxCullMode cullMode)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	Gnm::PrimitiveSetupCullFaceMode cullFaceMode = VuPs4GfxTypes::convert(cullMode);

	Gnm::PrimitiveSetup primSetup;
	primSetup.init();
	primSetup.setCullFace(cullFaceMode);
	gfxc.setPrimitiveSetup(primSetup);
}

//*****************************************************************************
bool VuPs4Gfx::setTexture(int sampler, VuBaseTexture *pBaseTexture)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	if ( pBaseTexture )
	{
		if ( pBaseTexture->isDerivedFrom(VuTexture::msRTTI) )
		{
			VuPs4Texture *pPs4Texture = static_cast<VuPs4Texture *>(pBaseTexture);

			gfxc.setTextures(Gnm::kShaderStagePs, sampler, 1, &pPs4Texture->mGnmTexture);
			gfxc.setSamplers(Gnm::kShaderStagePs, sampler, 1, &pPs4Texture->mGnmSampler);
		}
		else if ( pBaseTexture->isDerivedFrom(VuCubeTexture::msRTTI) )
		{
			VuPs4CubeTexture *pPs4CubeTexture = static_cast<VuPs4CubeTexture *>(pBaseTexture);

			gfxc.setTextures(Gnm::kShaderStagePs, sampler, 1, &pPs4CubeTexture->mGnmTexture);
			gfxc.setSamplers(Gnm::kShaderStagePs, sampler, 1, &pPs4CubeTexture->mGnmSampler);
		}
	}
	else
	{
		gfxc.setTextures(Gnm::kShaderStagePs, sampler, 1, VUNULL);
		gfxc.setSamplers(Gnm::kShaderStagePs, sampler, 1, VUNULL);
	}

	return true;
}

//*****************************************************************************
bool VuPs4Gfx::setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	if ( pShadowRenderTarget )
	{
		VuPs4ShadowRenderTarget *pPs4ShadowRenderTarget = static_cast<VuPs4ShadowRenderTarget *>(pShadowRenderTarget);

		gfxc.setTextures(Gnm::kShaderStagePs, sampler, 1, &pPs4ShadowRenderTarget->mGnmTexture);
		gfxc.setSamplers(Gnm::kShaderStagePs, sampler, 1, &pPs4ShadowRenderTarget->mGnmSampler);
	}
	else
	{
		gfxc.setTextures(Gnm::kShaderStagePs, sampler, 1, VUNULL);
		gfxc.setSamplers(Gnm::kShaderStagePs, sampler, 1, VUNULL);
	}

	return true;
}

//*****************************************************************************
void VuPs4Gfx::drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);

	// draw
	gfxc.setPrimitiveType(VuPs4GfxTypes::convert(primitiveType));
	gfxc.drawIndexAuto(vertexCount, startVertex, 0);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuPs4Gfx::drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	VUUINT16 *pIndexData = (VUUINT16 *)mpCurIndexBuffer->mpBuffer;

	gfxc.setPrimitiveType(VuPs4GfxTypes::convert(primitiveType));
	gfxc.drawIndex(vertexCount, pIndexData + startIndex);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuPs4Gfx::drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	int stride = 4;

	VuPs4VertexBuffer *pPs4VertexBuffer = static_cast<VuPs4VertexBuffer *>(pVertexColorBuffer);
	int vertexCount = pPs4VertexBuffer->mSize/stride;

	// set up vertex buffer
	Gnm::Buffer vertexBuffer;
	vertexBuffer.initAsVertexBuffer(pPs4VertexBuffer->mpBuffer, Gnm::kDataFormatR8G8B8A8Unorm, stride, vertexCount);

	int slot = mpCurVertexDeclaration->mStreams[0].mElementCount;
	gfxc.setVertexBuffers(Gnm::kShaderStageVs, slot, 1, &vertexBuffer);

	drawIndexedPrimitive(primitiveType, minIndex, numVerts, startIndex, primitiveCount);
}

//*****************************************************************************
void VuPs4Gfx::drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	if ( mpCurDisplayBuffer->mDynamicVBOffset + vertexCount*mCurVertexStride > PS4_DYNAMIC_VB_SIZE )
	{
		VUPRINTF("Dynamic VB overflow!\n");
		return;
	}

	// write to dynamic vertex buffer
	VUBYTE *pVerts = (VUBYTE *)mpCurDisplayBuffer->mpDynamicVB + mpCurDisplayBuffer->mDynamicVBOffset;
	memcpy(pVerts, pVertexData, vertexCount*mCurVertexStride);

	// set up vertex buffers
	const VuPs4VertexDeclaration::Stream &stream = mpCurVertexDeclaration->mStreams[0];
	Gnm::Buffer vertexBuffers[Gnm::kSlotCountVertexBuffer];
	for ( int i = 0; i < stream.mElementCount; i++ )
		vertexBuffers[i].initAsVertexBuffer(pVerts + stream.mElements[i].mOffset, stream.mElements[i].mDataFormat, stream.mStride, vertexCount);

	// draw
	gfxc.setVertexBuffers(Gnm::kShaderStageVs, 0, stream.mElementCount, vertexBuffers);
	gfxc.setPrimitiveType(VuPs4GfxTypes::convert(primitiveType));
	gfxc.drawIndexAuto(vertexCount);

	mpCurDisplayBuffer->mDynamicVBOffset += vertexCount*mCurVertexStride;
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuPs4Gfx::drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData)
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	if ( vertexCount > 32764 )
	{
		VUPRINTF("VuPs4Gfx::drawPrimitiveUP() overflow!\n");
		return;
	}

	if ( mpCurDisplayBuffer->mDynamicVBOffset + numVerts*mCurVertexStride > PS4_DYNAMIC_VB_SIZE )
	{
		VUPRINTF("Dynamic VB overflow!\n");
		return;
	}

	// write to dynamic vertex buffer
	VUBYTE *pVerts = (VUBYTE *)mpCurDisplayBuffer->mpDynamicVB + mpCurDisplayBuffer->mDynamicVBOffset;
	memcpy(pVerts, pVertexData, numVerts*mCurVertexStride);

	// set up vertex buffers
	const VuPs4VertexDeclaration::Stream &stream = mpCurVertexDeclaration->mStreams[0];
	Gnm::Buffer vertexBuffers[Gnm::kSlotCountVertexBuffer];
	for ( int i = 0; i < stream.mElementCount; i++ )
		vertexBuffers[i].initAsVertexBuffer(pVerts + stream.mElements[i].mOffset, stream.mElements[i].mDataFormat, stream.mStride, numVerts);

	// draw
	gfxc.setVertexBuffers(Gnm::kShaderStageVs, 0, stream.mElementCount, vertexBuffers);
	gfxc.setPrimitiveType(VuPs4GfxTypes::convert(primitiveType));
	gfxc.drawIndexInline(vertexCount, pIndexData, vertexCount*2);

	mpCurDisplayBuffer->mDynamicVBOffset += numVerts*mCurVertexStride;
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuPs4Gfx::resetStats()
{
	VuGfx::resetStats();

	mMaxDVBSize = VuMax(mMaxDVBSize, mCurDVBSize);
	mMaxDCBSize = VuMax(mMaxDCBSize, mCurDCBSize);
	mMaxCCBSize = VuMax(mMaxCCBSize, mCurCCBSize);
}

//*****************************************************************************
void VuPs4Gfx::printStats()
{
	VuGfx::printStats();

	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "Gfx" )
			{
				pPage->printf("DVB: size-%dK max-%dK cur-%dK\n", PS4_DYNAMIC_VB_SIZE/1024, mMaxDVBSize/1024, mCurDVBSize/1024);
				pPage->printf("DCB: size-%dK max-%dK cur-%dK\n", PS4_DRAW_COMMAND_BUFFER_SIZE/1024, mMaxDCBSize/1024, mCurDCBSize/1024);
				pPage->printf("CCB: size-%dK max-%dK cur-%dK\n", PS4_CONSTANT_COMMAND_BUFFER_SIZE/1024, mMaxCCBSize/1024, mCurCCBSize/1024);
			}
		}
	}
}

//*****************************************************************************
void VuPs4Gfx::updateState()
{
	Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

	if ( mpCurShaderProgram )
	{
		VuPs4VertexShader *pPs4VertexShader = static_cast<VuPs4VertexShader *>(mpCurShaderProgram->mapShaders[VuShaderProgram::VERTEX_SHADER]);
		for ( VUUINT32 i = 0; i < pPs4VertexShader->mConstantBufferCount; i++ )
		{
			if ( pPs4VertexShader->mConstantBufferDirtyBits & (1<<i) )
			{
				const VuPs4Shader::VuConstantBuffer &cb = pPs4VertexShader->mConstantBuffers[i];

				void *constants = gfxc.allocateFromCommandBuffer(cb.mSize, Gnm::kEmbeddedDataAlignment4);
				memcpy(constants, cb.mpData, cb.mSize);

				Gnm::Buffer constBuffer;
				constBuffer.initAsConstantBuffer(constants, cb.mSize);

				gfxc.setConstantBuffers(Gnm::kShaderStageVs, i, 1, &constBuffer);
			}
		}
		pPs4VertexShader->mConstantBufferDirtyBits = 0;

		VuPs4PixelShader *pPs4PixelShader = static_cast<VuPs4PixelShader *>(mpCurShaderProgram->mapShaders[VuShaderProgram::PIXEL_SHADER]);
		for ( VUUINT32 i = 0; i < pPs4PixelShader->mConstantBufferCount; i++ )
		{
			if ( pPs4PixelShader->mConstantBufferDirtyBits & (1<<i) )
			{
				const VuPs4Shader::VuConstantBuffer &cb = pPs4PixelShader->mConstantBuffers[i];

				void *constants = gfxc.allocateFromCommandBuffer(cb.mSize, Gnm::kEmbeddedDataAlignment4);
				memcpy(constants, cb.mpData, cb.mSize);

				Gnm::Buffer constBuffer;
				constBuffer.initAsConstantBuffer(constants, cb.mSize);

				gfxc.setConstantBuffers(Gnm::kShaderStagePs, i, 1, &constBuffer);
			}
		}
		pPs4PixelShader->mConstantBufferDirtyBits = 0;
	}
}

//*****************************************************************************
void VuPs4Gfx::doPendingResolve()
{
	if ( mpGnmPendingResolveRenderTarget )
	{
		Gnmx::GfxContext &gfxc = mpCurDisplayBuffer->mContext;

		gfxc.waitForGraphicsWrites(mpGnmPendingResolveRenderTarget->getBaseAddress256ByteBlocks(), mpGnmPendingResolveRenderTarget->getSliceSizeInBytes()>>8,
			Gnm::kWaitTargetSlotCb0, Gnm::kCacheActionWriteBackAndInvalidateL1andL2, Gnm::kExtendedCacheActionFlushAndInvalidateCbCache, 
			Gnm::kStallCommandBufferParserEnable);

		mpGnmPendingResolveRenderTarget = VUNULL;
	}
}

//*****************************************************************************
void *VuPs4Gfx::allocateOnionMemory(void *instance, uint32_t size, sce::Gnm::AlignmentType alignment)
{
	return VuPs4Gfx::IF()->mOnionMemoryManager.allocate(size, alignment);
}

//*****************************************************************************
void VuPs4Gfx::releaseOnionMemory(void *instance, void *pointer)
{
	// add to recycle bin (to be recycled after gpu is done)
	VuPs4Gfx::IF()->mOnionRecycleBin.push_back(pointer);
}

//*****************************************************************************
void *VuPs4Gfx::allocateGarlicMemory(void *instance, uint32_t size, sce::Gnm::AlignmentType alignment)
{
	return VuPs4Gfx::IF()->mGarlicMemoryManager.allocate(size, alignment);
}

//*****************************************************************************
void VuPs4Gfx::releaseGarlicMemory(void *instance, void *pointer)
{
	// add to recycle bin (to be recycled after gpu is done)
	VuPs4Gfx::IF()->mGarlicRecycleBin.push_back(pointer);
}
