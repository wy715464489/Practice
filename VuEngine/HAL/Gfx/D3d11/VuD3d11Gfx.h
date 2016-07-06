//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 interface class for Gfx.
//
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif
#include "VuD3d11GfxTypes.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuRect.h"

class VuD3d11ShaderProgram;


class VuD3d11Gfx : public VuGfx
{
public:
	VuD3d11Gfx();

	virtual bool init();
	virtual void release();

	// platform-specific functionality
	static VuD3d11Gfx *IF() { return static_cast<VuD3d11Gfx *>(VuGfx::IF()); }

	// get d3d device/context
	ID3D11Device		*getD3dDevice() { return mpDevice; }
	ID3D11DeviceContext	*getD3dDeviceContext() { return mpDeviceContext; }

	ID3D11BlendState	*createBlendState(const VuPipelineStateParams &params);

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
	virtual void		setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget);
	virtual void		setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer);
	virtual void		setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget);

	// begin/end
	virtual bool		beginScene(VUHANDLE hDisplay);
	virtual bool		endScene(VUHANDLE hDisplay);

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

	// sampler state
	virtual bool		setTexture(int sampler, VuBaseTexture *pBaseTexture);
	virtual bool		setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer);

	// primitive rendering
	virtual void		drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount);
	virtual void		drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount);
	virtual void		drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer);
	virtual void		drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData);
	virtual void		drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData);

	// stats
	virtual void		resetStats();
	virtual void		printStats();

protected:
	void				updateState();
	HRESULT				createDepthStencilView(int width, int height, DXGI_FORMAT format, ID3D11DepthStencilView **ppDepthStencil);

	ID3D11Device		*mpDevice;
	ID3D11DeviceContext	*mpDeviceContext;

	// state
	int						mCurRenderTargetWidth;
	int						mCurRenderTargetHeight;
	VuRect					mCurViewport;
	VUUINT					mCurVertexStride;
	VuD3d11ShaderProgram	*mpCurShaderProgram;

	// blend state
	struct VuBlendState
	{
		bool			mEnable;
		VuGfxBlendMode	mSrcMode;
		VuGfxBlendMode	mDstMode;
		bool			mColorWriteEnable;

		ID3D11BlendState	*mpD3dBlendState;
	};
	typedef std::vector<VuBlendState> BlendStates;
	BlendStates		mBlendStates;

	// rasterizer state
	struct VuRasterizerState
	{
		VuGfxCullMode	mCullMode;
		bool			mScissorTestEnable;

		ID3D11RasterizerState	*mpD3dRasterizerState;
	};
	typedef std::vector<VuRasterizerState> RasterizerStates;
	RasterizerStates	mRasterizerStates;
	bool				mRasterizerStateDirty;
	VuGfxCullMode		mCullMode;
	bool				mScissorTestEnable;

	// dynamic drawing
	ID3D11Buffer	*mpDynamicVB;
	ID3D11Buffer	*mpDynamicIB;
	VUUINT			mDynamicVBOffset;
	VUUINT			mDynamicIBOffset;
	VuVertexBuffer	*mpCurVB;
	VuIndexBuffer	*mpCurIB;
	bool			mDynamicVBActive;
	bool			mDynamicIBActive;

	// stats
	VUUINT			mCurDynamicVBSize;
	VUUINT			mCurDynamicIBSize;
	VUUINT			mMaxDynamicVBSize;
	VUUINT			mMaxDynamicIBSize;
};


// macros

#define D3DCALL(call)			\
{								\
	HRESULT res = call;			\
	VUD3DASSERT(res, #call);	\
}

#define D3DRELEASE(p)	\
{						\
	if ( p )			\
	{					\
		p->Release();	\
		p = NULL;		\
	}					\
}

#define VUD3DASSERT(res, msg)								\
{															\
	if ( res != S_OK )									\
	{														\
		VUWARNING("D3D Error %d (%x)\n\n%s\nline %d\n\n%s",	\
			res, res, __FILE__, __LINE__, msg);				\
	}														\
}
