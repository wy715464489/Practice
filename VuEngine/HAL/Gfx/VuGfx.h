//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Gfx library.
// 
//*****************************************************************************

#pragma once

#include "VuGfxTypes.h"
#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Util/VuColor.h"

class VuEngine;
class VuColor;
class VuRect;

struct VuSetRenderTargetParams;
struct VuPipelineStateParams;
struct VuDepthStencilStateParams;

class VuRenderTarget;
class VuDepthRenderTarget;
class VuShadowRenderTarget;
class VuFxRenderTarget;
class VuBaseTexture;
class VuTexture;
class VuCubeTexture;
class VuShaderProgram;
class VuVertexDeclaration;
class VuVertexBuffer;
class VuIndexBuffer;
class VuTextureState;
class VuVertexDeclarationParams;
class VuPipelineState;
class VuDepthStencilState;

class VuBinaryDataReader;


class VuGfx : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuGfx)

protected:
	VuGfx();

	// called by engine
	friend class VuEngine;
	virtual bool init(VUHANDLE hWindow, VUHANDLE hDevice) { return true; }
	virtual void postInit();
	virtual void preRelease();

public:
	// configuration
	void				setSyncGPU(bool syncGPU)		{ mSyncGPU = syncGPU; }
	bool				getSyncGPU()					{ return mSyncGPU; }
	void				setFlipInterval(int flipInterval)	{ mFlipInterval = flipInterval; }
	int					getFlipInterval()					{ return mFlipInterval; }

	// thread ownership (not required)
	virtual void		acquireThreadOwnership() {}
	virtual void		releaseThreadOwnership() {}

	// set focus window (not required)
	virtual void		setFocusWindow(VUHANDLE hWndFocus) {}

	// display size
	virtual void		resize(VUHANDLE hDisplay, int width, int height) = 0;
	virtual void		getDisplaySize(VUHANDLE hDisplay, int &width, int &height) = 0;

	// resources
	virtual VuRenderTarget			*createRenderTarget(int width, int height) = 0;
	virtual VuDepthRenderTarget		*createDepthRenderTarget(int width, int height) = 0;
	virtual VuShadowRenderTarget	*createShadowRenderTarget(int width, int height, int count) = 0;
	virtual VuFxRenderTarget		*createFxRenderTarget(int width, int height, VuGfxFormat format) = 0;

	virtual VuVertexBuffer			*createVertexBuffer(int size, VUUINT32 usageFlags) = 0;
	virtual VuIndexBuffer			*createIndexBuffer(int count, VUUINT32 usageFlags) = 0;
	virtual VuVertexDeclaration		*createVertexDeclaration(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram) = 0;

	virtual VuTexture				*createTexture(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state) = 0;
	virtual VuTexture				*loadTexture(VuBinaryDataReader &reader, int skipLevels) = 0;
	virtual VuCubeTexture			*loadCubeTexture(VuBinaryDataReader &reader, int skipLevels) = 0;
	virtual VuShaderProgram			*loadShaderProgram(VuBinaryDataReader &reader) = 0;

	// states
	virtual VuPipelineState			*createPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params) = 0;
	virtual VuDepthStencilState		*createDepthStencilState(const VuDepthStencilStateParams &params) = 0;

	// render targets
	virtual void		getCurRenderTargetSize(int &width, int &height) = 0;
	virtual void		setRenderTarget(const VuSetRenderTargetParams &params) = 0;
	virtual void		setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget) = 0;
	virtual void		setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer) = 0;
	virtual void		setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget) = 0;

	// additional displays (not required)
	virtual VUHANDLE	createDisplay(VUHANDLE hWnd) { return VUNULL; }
	virtual void		releaseDisplay(VUHANDLE hDisplay) {}

	// begin/end/present
	virtual bool		beginScene(VUHANDLE hDisplay) { return true; }
	virtual bool		endScene(VUHANDLE hDisplay) { return true; }

	// full screen effect hint (not required)
	virtual void		beginFullScreenEffect() {}
	virtual void		endFullScreenEffect() {}

	// clear
	virtual bool		clear(VUUINT32 flags, const VuColor &color, float depth) = 0;

	// viewport
	virtual const VuRect	&getViewport() const = 0;
	virtual bool			setViewport(const VuRect &rect) = 0;

	// scissor rect
	virtual bool			setScissorRect(const VuRect *pRect) = 0;

	// clip planes
	virtual void			setClipPlane(const VuVector4 &clipPlane)	{ mClipPlane = clipPlane; }
	virtual const VuVector4	&getClipPlane(void)							{ return mClipPlane; }

	// resource state
	virtual bool		setVertexBuffer(VuVertexBuffer *pVertexBuffer) = 0;
	virtual bool		setIndexBuffer(VuIndexBuffer *pIndexBuffer) = 0;

	// render state
	virtual void		setPipelineState(VuPipelineState *pPipelineState) = 0;
	virtual void		setDepthStencilState(VuDepthStencilState *pDepthStencilState) = 0;
	virtual void		setCullMode(VuGfxCullMode cullMode) = 0;

	// sampler state
	virtual bool		setTexture(int sampler, VuBaseTexture *pBaseTexture) = 0;
	virtual bool		setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer) = 0;

	// primitive rendering
	virtual void		drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount) = 0;
	virtual void		drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount) = 0;
	virtual void		drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer) = 0;
	virtual void		drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData) = 0;
	virtual void		drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData) = 0;

	// allow for post-draw synchronization
	virtual void		syncPreDraw() {}
	virtual void		syncPostDraw() {}

	// capabilities
	virtual bool		supportsSSAO() { return false; }
	virtual bool		supportsHBAO() { return false; }
	static bool			supportsVertexDeclType(const std::string &platform, eGfxDeclType declType);
	static bool			supportsTextureFormat(const std::string &platform, VuGfxFormat format);

	// stats
	virtual void		resetStats();
	virtual void		printStats();

	virtual int			getPrevPrimitiveCount() { return mPrevPrimitiveCount; }
	virtual int			getPrevDrawCallCount() { return mPrevDrawCallCount; }

protected:
	void				configFlipInterval(int value) { mFlipInterval = value; }

	static int			calcVertexCount(VuGfxPrimitiveType primitiveType, int primitiveCount);

	// configuration
	int					mFlipInterval;
	bool				mSyncGPU;

	// stats
	int					mPrimitiveCount;
	int					mDrawCallCount;
	int					mPrevPrimitiveCount;
	int					mPrevDrawCallCount;
	int					mMaxPrimitiveCount;
	int					mMaxDrawCallCount;
	VuVector4			mClipPlane;
};
