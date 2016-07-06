//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 interface class for Gfx.
//
//*****************************************************************************

#pragma once

#include <gnm.h>
#include <gnmx.h>

#include "toolkit/allocator.h"

#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Memory/VuMemoryManager.h"

class VuPs4PipelineState;
class VuPs4ShaderProgram;
class VuPs4VertexDeclaration;
class VuPs4IndexBuffer;
class VuPs4RenderTarget;

using namespace sce;


class VuPs4Gfx : public VuGfx
{
protected:
	virtual bool init(VUHANDLE hWindow, VUHANDLE hDevice);
	virtual void release();

public:
	VuPs4Gfx();
	~VuPs4Gfx();

	// cross-platform functionality

	// display size
	virtual void		resize(VUHANDLE hDisplay, int width, int height);
	virtual void		getDisplaySize(VUHANDLE hDisplay, int &width, int &height);

	// resources
	virtual VuRenderTarget			*createRenderTarget(int width, int height);
	virtual VuDepthRenderTarget		*createDepthRenderTarget(int width, int height);
	virtual VuShadowRenderTarget	*createShadowRenderTarget(int width, int height, int count);
	virtual VuFxRenderTarget		*createFxRenderTarget(int width, int height, VuGfxFormat format);

	virtual VuVertexBuffer			*createVertexBuffer(int size, VUUINT32 usageFlags);
	virtual VuIndexBuffer			*createIndexBuffer(int count, VUUINT32 usageFlags);
	virtual VuVertexDeclaration		*createVertexDeclaration(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram);

	virtual VuTexture				*createTexture(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state);
	virtual VuTexture				*loadTexture(VuBinaryDataReader &reader, int skipLevels);
	virtual VuCubeTexture			*loadCubeTexture(VuBinaryDataReader &reader, int skipLevels);
	virtual VuShaderProgram			*loadShaderProgram(VuBinaryDataReader &reader);

	// states
	virtual VuPipelineState			*createPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	virtual VuDepthStencilState		*createDepthStencilState(const VuDepthStencilStateParams &params);

	// render targets
	virtual void		getCurRenderTargetSize(int &width, int &height);
	virtual void		setRenderTarget(const VuSetRenderTargetParams &params);
	virtual void		setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget);
	virtual void		setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer);
	virtual void		setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget);

	// begin/end/present
	virtual bool		beginScene(VUHANDLE hDisplay);
	virtual bool		endScene(VUHANDLE hDisplay);

	// clear
	virtual bool		clear(VUUINT32 flags, const VuColor &color, float depth);

	// viewports
	virtual const VuRect	&getViewport() const { return mCurViewport; }
	virtual bool			setViewport(const VuRect &rect);

	// scissor rect
	virtual bool			setScissorRect(const VuRect *pRect);

	// resource state
	virtual bool		setVertexBuffer(VuVertexBuffer *pVertexBuffer);
	virtual bool		setIndexBuffer(VuIndexBuffer *pIndexBuffer);

	// render state
	virtual void		setPipelineState(VuPipelineState *pPipelineState);
	virtual void		setDepthStencilState(VuDepthStencilState *pDepthStencilState);
	virtual void		setCullMode(VuGfxCullMode cullMode);

	// sampler state
	virtual bool		setTexture(int sampler, VuBaseTexture *pBaseTexture);
	virtual bool		setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer);

	// primitive rendering
	virtual void		drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount);
	virtual void		drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount);
	virtual void		drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer);
	virtual void		drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData);
	virtual void		drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData);

	// capabilities
	virtual bool		supportsSSAO() { return true; }
	virtual bool		supportsHBAO() { return true; }

	// stats
	virtual void		resetStats();
	virtual void		printStats();

	// platform-specific functionality
	static VuPs4Gfx *IF() { return static_cast<VuPs4Gfx *>(VuGfx::IF()); }

	// gpu memory management
	sce::Gnmx::Toolkit::IAllocator	&onionAllocator() { return mOnionMemoryAllocator; }
	sce::Gnmx::Toolkit::IAllocator	&garlicAllocator() { return mGarlicMemoryAllocator; }

	// gnmx gfx context
	Gnmx::GfxContext	&getCurGfxContext() { return mpCurDisplayBuffer->mContext; }

protected:
	void				updateState();
	void				doPendingResolve();

	static void			*allocateOnionMemory(void *instance, uint32_t size, sce::Gnm::AlignmentType alignment);
	static void			releaseOnionMemory(void *instance, void *pointer);
	static void			*allocateGarlicMemory(void *instance, uint32_t size, sce::Gnm::AlignmentType alignment);
	static void			releaseGarlicMemory(void *instance, void *pointer);

	VuMemoryManager				mOnionMemoryManager;
	Gnmx::Toolkit::IAllocator	mOnionMemoryAllocator;
	VuMemoryManager				mGarlicMemoryManager;
	Gnmx::Toolkit::IAllocator	mGarlicMemoryAllocator;

	struct DisplayBuffer
	{
		Gnmx::GfxContext		mContext;
		void					*mCueCpRamShadowBuffer;
		void					*mCueHeapAddr;
		void					*mDrawCommandBuffer;
		void					*mConstantCommandBuffer;
		Gnm::RenderTarget		mRenderTarget;
		Gnm::DepthRenderTarget	mDepthTarget;
		void					*mpDynamicVB;
		VUUINT					mDynamicVBOffset;
	};
	void				**mpSurfaceAddresses;
	DisplayBuffer		*mpDisplayBuffers;
	int					mCurDisplayBufferIndex;
	DisplayBuffer		*mpCurDisplayBuffer;
	int					mFrameIndex;
	SceKernelEqueue		mEopEventQueue;
	int32_t				mVideoOutHandle;

	// shader program / vertex declaration
	VuPs4PipelineState		*mpCurPipelineState;
	VuPs4ShaderProgram		*mpCurShaderProgram;
	VuPs4VertexDeclaration	*mpCurVertexDeclaration;
	VuPs4IndexBuffer		*mpCurIndexBuffer;

	// state
	int					mDisplayWidth;
	int					mDisplayHeight;
	int					mCurRenderTargetWidth;
	int					mCurRenderTargetHeight;
	VuRect				mCurViewport;
	VUUINT				mCurVertexStride;

	// gpu memory recycle bin
	typedef std::deque<void *> MemoryRecycleBin;
	MemoryRecycleBin	mGarlicRecycleBin;
	MemoryRecycleBin	mOnionRecycleBin;

	// current render target
	Gnm::RenderTarget		*mpCurGnmColorRenderTarget;
	Gnm::DepthRenderTarget	*mpCurGnmDepthRenderTarget;

	// render target that needs to be resolved
	Gnm::RenderTarget		*mpGnmPendingResolveRenderTarget;

	// stats
	VUUINT			mCurDVBSize;
	VUUINT			mMaxDVBSize;
	VUUINT			mCurDCBSize;
	VUUINT			mMaxDCBSize;
	VUUINT			mCurCCBSize;
	VUUINT			mMaxCCBSize;
};
