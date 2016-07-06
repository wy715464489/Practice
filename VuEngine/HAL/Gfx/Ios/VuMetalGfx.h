//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Ios Gfx Metal HAL.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuRect.h"

class VuMetalIndexBuffer;
class VuMetalShaderProgram;


class VuMetalGfx : public VuGfx
{
public:
	VuMetalGfx();
	
	virtual bool		init(VUHANDLE hWindow, VUHANDLE hDevice);
	virtual void		release();
	
	// thread ownership (not required)
	virtual void		acquireThreadOwnership() {}
	virtual void		releaseThreadOwnership() {}
	
	// set focus window (not required)
	virtual void		setFocusWindow(VUHANDLE hWndFocus) {}
	
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
	
	// additional displays (not required)
	virtual VUHANDLE	createDisplay(VUHANDLE hWnd) { return VUNULL; }
	virtual void		releaseDisplay(VUHANDLE hDisplay) {}
	
	// begin/end/present
	virtual bool		beginScene(VUHANDLE hDisplay);
	virtual bool		endScene(VUHANDLE hDisplay);
	
	// full screen effect hint (not required)
	virtual void		beginFullScreenEffect() {}
	virtual void		endFullScreenEffect() {}
	
	// clear
	virtual bool		clear(VUUINT32 flags, const VuColor &color, float depth);
	
	// viewport
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
	
	// texture state
	virtual bool		setTexture(int sampler, VuBaseTexture *pBaseTexture);
	virtual bool		setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer);
	
	// primitive rendering
	virtual void		drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount);
	virtual void		drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount);
	virtual void		drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer);
	virtual void		drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData);
	virtual void		drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData);
	
	// allow for post-draw synchronization
	virtual void		syncPreDraw();
	virtual void		syncPostDraw();
	
	// stats
	virtual void		resetStats();
	virtual void		printStats();
	
	// platform-specific functionality
	static bool				checkForMetal();
	static id <MTLDevice>	getDevice();
	static void				setMetalLayer(CAMetalLayer *pMetalLayer, float scaleFactor);
	
	static int				calcVertexCount(VuGfxPrimitiveType primitiveType, int primitiveCount);
	
protected:
	void					updateState();
	
	enum { IN_FLIGHT_COMMAND_BUFFER_COUNT = 2 };
	enum { DYNAMIC_BUFFER_SIZE = 2*1024*1024 };
	
	int						mDisplayWidth;
	int						mDisplayHeight;
	int						mCurRenderTargetWidth;
	int						mCurRenderTargetHeight;
	VuRect					mCurViewport;
	VuMetalIndexBuffer		*mpCurIndexBuffer;
	VuMetalShaderProgram	*mpCurShaderProgram;
	int						mCurVertexStride;

	dispatch_semaphore_t	mInflightSemaphore;
	id <MTLCommandQueue>	mCommandQueue;
	id <MTLTexture>			mDepthTexture;
	id <MTLBuffer>			mDynamicBuffers[IN_FLIGHT_COMMAND_BUFFER_COUNT];
	int						mDynamicBufferIndex;
	int						mDynamicBufferOffset;
	
	// only valid between begin/end pair
	id <CAMetalDrawable>			mDrawable;
	id <MTLCommandBuffer>			mCommandBuffer;
	id <MTLRenderCommandEncoder>	mEncoder;
	
	// stats
	int						mCurDynamicBufferSize;
	int						mMaxDynamicBufferSize;
};
