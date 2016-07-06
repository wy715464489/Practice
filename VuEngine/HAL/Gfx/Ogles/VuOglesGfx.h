//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES interface class for Gfx.
//
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuRect.h"


class VuOglesVertexDeclaration;
class VuOglesVertexBuffer;
class VuOglesIndexBuffer;


class VuOglesGfx : public VuGfx
{
protected:
	virtual bool init(VUHANDLE hWindow, VUHANDLE hDevice);
	virtual void release();

public:
	VuOglesGfx();
	~VuOglesGfx();

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
	virtual void		setRenderTarget(const VuSetRenderTargetParams &params) = 0;
	virtual void		setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget);
	virtual void		setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer);
	virtual void		setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget);

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

	// platform-specific functionality
	static VuOglesGfx *IF() { return static_cast<VuOglesGfx *>(VuGfx::IF()); }

	// info
	int					getGlVersion() { return mGlVersion; }
	bool				getExtension(const char *name);
	bool				getDxtCompression() { return mDxtCompression; }
	bool				getPvrtcCompression() { return mPvrtcCompression; }

	void				setContextDestroyed()	{ mContextDestroyed = true; }
	bool				getContextDestroyed()	{ return mContextDestroyed; }
	
	void				setDefaultFramebuffer(int framebuffer) { mDefaultFramebuffer = framebuffer; }
	int					getDefaultFramebuffer()                { return mDefaultFramebuffer; }
	
	void				setDisplayWidth(int width)   { mDisplayWidth = mCurRenderTargetWidth = width; }
	void				setDisplayHeight(int height) { mDisplayHeight = mCurRenderTargetHeight = height; }

	void				bindVertexBuffer(unsigned int buffer);
	void				bindIndexBuffer(unsigned int buffer);

	void				checkFramebufferStatus();

protected:
	// state
	int							mDisplayWidth;
	int							mDisplayHeight;
	int							mCurRenderTargetWidth;
	int							mCurRenderTargetHeight;
	VuRect						mCurViewport;
	VuOglesVertexDeclaration	*mpCurVertexDeclaration;
	VuOglesVertexBuffer			*mpCurVertexBuffer;
	VuOglesIndexBuffer			*mpCurIndexBuffer;
	VUUINT32					mPrevVertexBufferId;
	VUUINT32					mPrevIndexBufferId;
	VUUINT32					mCurVertexStride;

	typedef std::set<std::string> Extensions;
	int					mGlVersion;
	Extensions			mExtensions;
	bool				mDxtCompression;
	bool				mPvrtcCompression;
	bool				mContextDestroyed;
	int					mDefaultFramebuffer;
};
