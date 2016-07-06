//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 interface class for Gfx.
//
//*****************************************************************************

#include "VuD3d11Gfx.h"
#include "VuD3d11ShaderProgram.h"
#include "VuD3d11VertexDeclaration.h"
#include "VuD3d11PipelineState.h"
#include "VuD3d11DepthStencilState.h"
#include "VuD3d11RenderTarget.h"
#include "VuD3d11DepthRenderTarget.h"
#include "VuD3d11ShadowRenderTarget.h"
#include "VuD3d11FxRenderTarget.h"
#include "VuD3d11VertexBuffer.h"
#include "VuD3d11IndexBuffer.h"
#include "VuD3d11Texture.h"
#include "VuD3d11CubeTexture.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Dev/VuDevStat.h"


// constants
#ifdef VURETAIL
	#define DYNAMIC_VB_SIZE (4*1024*1024)
	#define DYNAMIC_IB_SIZE (512*1024)
#else
	#define DYNAMIC_VB_SIZE (16*1024*1024)
	#define DYNAMIC_IB_SIZE (4*512*1024)
#endif


//*****************************************************************************
VuD3d11Gfx::VuD3d11Gfx():
	mpDevice(VUNULL),
	mCurRenderTargetWidth(0),
	mCurRenderTargetHeight(0),
	mCurViewport(0,0,1,1),
	mCurVertexStride(0),
	mpCurShaderProgram(VUNULL),
	mRasterizerStateDirty(true),
	mCullMode(VUGFX_CULL_CW),
	mScissorTestEnable(false),
	mDynamicVBOffset(0),
	mDynamicIBOffset(0),
	mDynamicVBActive(false),
	mDynamicIBActive(false),
	mCurDynamicVBSize(0),
	mCurDynamicIBSize(0),
	mMaxDynamicVBSize(0),
	mMaxDynamicIBSize(0)
{
}

//*****************************************************************************
bool VuD3d11Gfx::init()
{
	updateState();

	// create dynamic vertex/index buffers
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = DYNAMIC_VB_SIZE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		D3DCALL(mpDevice->CreateBuffer(&desc, NULL, &mpDynamicVB));

		desc.ByteWidth = DYNAMIC_IB_SIZE;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3DCALL(mpDevice->CreateBuffer(&desc, NULL, &mpDynamicIB));
	}

	return true;
}

//*****************************************************************************
void VuD3d11Gfx::release()
{
	for ( VUUINT i = 0; i < mBlendStates.size(); i++ )
		mBlendStates[i].mpD3dBlendState->Release();
	mBlendStates.clear();

	for ( VUUINT i = 0; i < mRasterizerStates.size(); i++ )
		mRasterizerStates[i].mpD3dRasterizerState->Release();
	mRasterizerStates.clear();

	mpDynamicVB->Release();
	mpDynamicIB->Release();
}

//*****************************************************************************
ID3D11BlendState *VuD3d11Gfx::createBlendState(const VuPipelineStateParams &params)
{
	// search existing states for match
	ID3D11BlendState *pBlendState = VUNULL;
	for ( BlendStates::iterator iter = mBlendStates.begin(); iter != mBlendStates.end(); iter++ )
		if ( iter->mEnable == params.mAlphaBlendEnabled && iter->mSrcMode == params.mSrcBlendMode && iter->mDstMode == params.mDstBlendMode && iter->mColorWriteEnable == params.mColorWriteEnabled )
			return iter->mpD3dBlendState;

	// if not found, add new state
	CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
	desc.RenderTarget[0].BlendEnable = params.mAlphaBlendEnabled;
	desc.RenderTarget[0].SrcBlend = VuD3d11GfxTypes::convert(params.mSrcBlendMode);
	desc.RenderTarget[0].DestBlend = VuD3d11GfxTypes::convert(params.mDstBlendMode);
	desc.RenderTarget[0].RenderTargetWriteMask = params.mColorWriteEnabled ? D3D11_COLOR_WRITE_ENABLE_ALL : 0;

	D3DCALL(mpDevice->CreateBlendState(&desc, &pBlendState));

	VuBlendState blendState;
	blendState.mEnable = params.mAlphaBlendEnabled;
	blendState.mSrcMode = params.mSrcBlendMode;
	blendState.mDstMode = params.mDstBlendMode;
	blendState.mColorWriteEnable = params.mColorWriteEnabled;
	blendState.mpD3dBlendState = pBlendState;
	mBlendStates.push_back(blendState);

	return blendState.mpD3dBlendState;
}

//*****************************************************************************
VuRenderTarget *VuD3d11Gfx::createRenderTarget(int width, int height)
{
	return VuD3d11RenderTarget::create(width, height);
}

//*****************************************************************************
VuDepthRenderTarget *VuD3d11Gfx::createDepthRenderTarget(int width, int height)
{
	return VuD3d11DepthRenderTarget::create(width, height);
}

//*****************************************************************************
VuShadowRenderTarget *VuD3d11Gfx::createShadowRenderTarget(int width, int height, int count)
{
	return VuD3d11ShadowRenderTarget::create(width, height, count);
}

//*****************************************************************************
VuFxRenderTarget *VuD3d11Gfx::createFxRenderTarget(int width, int height, VuGfxFormat format)
{
	return VuD3d11FxRenderTarget::create(width, height, format);
}

//*****************************************************************************
VuVertexBuffer *VuD3d11Gfx::createVertexBuffer(int size, VUUINT32 usageFlags)
{
	return VuD3d11VertexBuffer::create(size, usageFlags);
}

//*****************************************************************************
VuIndexBuffer *VuD3d11Gfx::createIndexBuffer(int count, VUUINT32 usageFlags)
{
	return VuD3d11IndexBuffer::create(count, usageFlags);
}

//*****************************************************************************
VuVertexDeclaration *VuD3d11Gfx::createVertexDeclaration(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	return VuD3d11VertexDeclaration::create(params, pShaderProgram);
}

//*****************************************************************************
VuTexture *VuD3d11Gfx::createTexture(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	return VuD3d11Texture::create(width, height, usageFlags, format, state);
}

//*****************************************************************************
VuTexture *VuD3d11Gfx::loadTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuD3d11Texture::load(reader, skipLevels);
}

//*****************************************************************************
VuCubeTexture *VuD3d11Gfx::loadCubeTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuD3d11CubeTexture::load(reader, skipLevels);
}

//*****************************************************************************
VuShaderProgram *VuD3d11Gfx::loadShaderProgram(VuBinaryDataReader &reader)
{
	return VuD3d11ShaderProgram::load(reader);
}

//*****************************************************************************
VuPipelineState *VuD3d11Gfx::createPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	return VuD3d11PipelineState::create(pSP, pVD, params);
}

//*****************************************************************************
VuDepthStencilState *VuD3d11Gfx::createDepthStencilState(const VuDepthStencilStateParams &params)
{
	return VuD3d11DepthStencilState::create(params);
}

//*****************************************************************************
void VuD3d11Gfx::getCurRenderTargetSize(int &width, int &height)
{
	width = mCurRenderTargetWidth;
	height = mCurRenderTargetHeight;
}

//*****************************************************************************
void VuD3d11Gfx::setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget)
{
	VuD3d11DepthRenderTarget *pD3d11DepthRenderTarget = (VuD3d11DepthRenderTarget *)pDepthRenderTarget;

	ID3D11RenderTargetView *pRenderTargetView = pD3d11DepthRenderTarget->mpD3d11ColorView;
	ID3D11DepthStencilView *pDepthStencilView = pD3d11DepthRenderTarget->mpD3d11DepthStencilView;

	mCurRenderTargetWidth = pD3d11DepthRenderTarget->getWidth();
	mCurRenderTargetHeight = pD3d11DepthRenderTarget->getHeight();

	mpDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	setViewport(VuRect(0, 0, 1, 1));

	VuVector4 vColor(1,1,1,1);
	mpDeviceContext->ClearRenderTargetView(pRenderTargetView, &vColor.mX);
	mpDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//*****************************************************************************
void VuD3d11Gfx::setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	VuD3d11ShadowRenderTarget *pD3d11ShadowRenderTarget = (VuD3d11ShadowRenderTarget *)pShadowRenderTarget;

	ID3D11RenderTargetView *pRenderTargetView = pD3d11ShadowRenderTarget->mColorBuffers[layer].mpD3d11View;
	ID3D11DepthStencilView *pDepthStencilView = pD3d11ShadowRenderTarget->mpD3d11DepthStencilView;

	mCurRenderTargetWidth = pD3d11ShadowRenderTarget->getWidth();
	mCurRenderTargetHeight = pD3d11ShadowRenderTarget->getHeight();

	mpDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	setViewport(VuRect(0,0,1,1));

	VuVector4 vColor(1,1,1,1);
	mpDeviceContext->ClearRenderTargetView(pRenderTargetView, &vColor.mX);
	mpDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//*****************************************************************************
void VuD3d11Gfx::setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget)
{
	VuD3d11FxRenderTarget *pD3d11FxRenderTarget = (VuD3d11FxRenderTarget *)pFxRenderTarget;

	ID3D11RenderTargetView *pRenderTargetView = pD3d11FxRenderTarget->mpD3d11ColorView;

	mCurRenderTargetWidth = pD3d11FxRenderTarget->getWidth();
	mCurRenderTargetHeight = pD3d11FxRenderTarget->getHeight();

	mpDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

	setViewport(VuRect(0, 0, 1, 1));
}

//*****************************************************************************
bool VuD3d11Gfx::beginScene(VUHANDLE hDisplay)
{
	mDynamicVBOffset = 0;
	mDynamicIBOffset = 0;

	if (!VuGfx::beginScene(hDisplay))
		return false;

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::endScene(VUHANDLE hDisplay)
{
	if (!VuGfx::endScene(hDisplay))
		return false;

	mCurDynamicVBSize = VuMax(mCurDynamicVBSize, mDynamicVBOffset);
	mCurDynamicIBSize = VuMax(mCurDynamicIBSize, mDynamicIBOffset);

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::clear(VUUINT32 flags, const VuColor &color, float depth)
{
	ID3D11RenderTargetView *pRenderTargetView = NULL;
	ID3D11DepthStencilView *pDepthStencilView = NULL;
	mpDeviceContext->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);

	if ( (flags & VUGFX_CLEAR_COLOR) && pRenderTargetView )
	{
		VuVector4 vColor;
		color.toVector4(vColor);
		mpDeviceContext->ClearRenderTargetView(pRenderTargetView, &vColor.mX);
	}

	if ( (flags & VUGFX_CLEAR_DEPTH) && pDepthStencilView )
	{
		mpDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, depth, 0);
	}

	D3DRELEASE(pRenderTargetView);
	D3DRELEASE(pDepthStencilView);

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::setViewport(const VuRect &rect)
{
	VuRect screenRect = rect*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);

	D3D11_VIEWPORT vp;

	vp.TopLeftX = float(VuRound(screenRect.mX));
	vp.TopLeftY = float(VuRound(screenRect.mY));
	vp.Width = float(VuRound(screenRect.mWidth));
	vp.Height = float(VuRound(screenRect.mHeight));
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	mpDeviceContext->RSSetViewports(1, &vp);

	mCurViewport = rect;

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::setScissorRect(const VuRect *pRect)
{
	bool enable = false;
	if ( pRect )
	{
		VuRect screenRect = (*pRect)*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);

		D3D11_RECT rc;
	
		rc.left = VuRound(screenRect.getLeft());
		rc.right = VuRound(screenRect.getRight());
		rc.top = VuRound(screenRect.getTop());
		rc.bottom = VuRound(screenRect.getBottom());

		mpDeviceContext->RSSetScissorRects(1, &rc);

		enable = true;
	}

	if ( enable != mScissorTestEnable )
	{
		mScissorTestEnable = enable;
		mRasterizerStateDirty = true;
	}

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::setVertexBuffer(VuVertexBuffer *pVertexBuffer)
{
	VuD3d11VertexBuffer *pD3d11VertexBuffer = static_cast<VuD3d11VertexBuffer *>(pVertexBuffer);

	VUUINT offset = 0;
	mpDeviceContext->IASetVertexBuffers(0, 1, &pD3d11VertexBuffer->mpD3dVertexBuffer, &mCurVertexStride, &offset);

	mDynamicVBActive = false;
	mpCurVB = pVertexBuffer;

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::setIndexBuffer(VuIndexBuffer *pIndexBuffer)
{
	VuD3d11IndexBuffer *pD3d11IndexBuffer = static_cast<VuD3d11IndexBuffer *>(pIndexBuffer);

	mpDeviceContext->IASetIndexBuffer(pD3d11IndexBuffer->mpD3dIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	mDynamicIBActive = false;
	mpCurIB = pIndexBuffer;

	return true;
}

//*****************************************************************************
void VuD3d11Gfx::setPipelineState(VuPipelineState *pPipelineState)
{
	VuD3d11PipelineState *pD3dPipelineState = (VuD3d11PipelineState *)pPipelineState;

	mpCurShaderProgram = (VuD3d11ShaderProgram *)pD3dPipelineState->mpShaderProgram;

	VuD3d11Shader *pVertexShader = mpCurShaderProgram->mapShaders[VuD3d11ShaderProgram::VERTEX_SHADER];
	mpDeviceContext->VSSetShader(pD3dPipelineState->mpD3dVertexShader, NULL, 0);
	mpDeviceContext->VSSetConstantBuffers(0, pVertexShader->mConstantBufferCount, pVertexShader->mapD3d11ConstantBuffers);

	VuD3d11Shader *pPixelShader = mpCurShaderProgram->mapShaders[VuD3d11ShaderProgram::PIXEL_SHADER];
	mpDeviceContext->PSSetShader(pD3dPipelineState->mpD3dPixelShader, NULL, 0);
	mpDeviceContext->PSSetConstantBuffers(0, pPixelShader->mConstantBufferCount, pPixelShader->mapD3d11ConstantBuffers);

	mpDeviceContext->IASetInputLayout(pD3dPipelineState->mpD3dInputLayout);

	mpDeviceContext->OMSetBlendState(pD3dPipelineState->mpD3dBlendState, NULL, 0xffffffff);

	mCurVertexStride = pPipelineState->mpVertexDeclaration->mParams.mStreams[0].mStride;
}

//*****************************************************************************
void VuD3d11Gfx::setDepthStencilState(VuDepthStencilState *pDepthStencilState)
{
	VuD3d11DepthStencilState *pD3dDepthStencilState = (VuD3d11DepthStencilState *)pDepthStencilState;
	mpDeviceContext->OMSetDepthStencilState(pD3dDepthStencilState->mpD3dDepthStencilState, 0);
}

//*****************************************************************************
void VuD3d11Gfx::setCullMode(VuGfxCullMode cullMode)
{
	mRasterizerStateDirty = true;
	mCullMode = cullMode;
}

//*****************************************************************************
bool VuD3d11Gfx::setTexture(int sampler, VuBaseTexture *pBaseTexture)
{
	if (pBaseTexture)
	{
		if (pBaseTexture->isDerivedFrom(VuTexture::msRTTI))
		{
			VuD3d11Texture *pWin32Texture = static_cast<VuD3d11Texture *>(pBaseTexture);

			mpDeviceContext->PSSetSamplers((sampler & 0xffff), 1, &pWin32Texture->mpD3dSamplerState);
			mpDeviceContext->PSSetShaderResources((sampler >> 16), 1, &pWin32Texture->mpD3dResourceView);
		}
		else if (pBaseTexture->isDerivedFrom(VuCubeTexture::msRTTI))
		{
			VuD3d11CubeTexture *pWin32CubeTexture = static_cast<VuD3d11CubeTexture *>(pBaseTexture);

			mpDeviceContext->PSSetSamplers((sampler & 0xffff), 1, &pWin32CubeTexture->mpD3dSamplerState);
			mpDeviceContext->PSSetShaderResources((sampler >> 16), 1, &pWin32CubeTexture->mpD3dResourceView);
		}
	}
	else
	{
		ID3D11ShaderResourceView *pSRV = NULL;
		mpDeviceContext->PSSetShaderResources(sampler, 1, &pSRV);

		ID3D11SamplerState *pSS = NULL;
		mpDeviceContext->PSSetSamplers(sampler, 1, &pSS);
	}

	return true;
}

//*****************************************************************************
bool VuD3d11Gfx::setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	if (pShadowRenderTarget)
	{
		VuD3d11ShadowRenderTarget *pWin32ShadowRenderTarget = static_cast<VuD3d11ShadowRenderTarget *>(pShadowRenderTarget);

		VuD3d11Texture *pWin32Texture = pWin32ShadowRenderTarget->mColorBuffers[layer].mpTexture;

		mpDeviceContext->PSSetSamplers((sampler & 0xffff), 1, &pWin32Texture->mpD3dSamplerState);
		mpDeviceContext->PSSetShaderResources((sampler >> 16), 1, &pWin32Texture->mpD3dResourceView);
	}
	else
	{
		ID3D11ShaderResourceView *pSRV = NULL;
		mpDeviceContext->PSSetShaderResources(sampler, 1, &pSRV);

		ID3D11SamplerState *pSS = NULL;
		mpDeviceContext->PSSetSamplers(sampler, 1, &pSS);
	}

	return true;
}

//*****************************************************************************
void VuD3d11Gfx::drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount)
{
	if ( mDynamicVBActive )
		setVertexBuffer(mpCurVB);

	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);

	mpDeviceContext->IASetPrimitiveTopology(VuD3d11GfxTypes::convert(primitiveType));
	mpDeviceContext->Draw(vertexCount, startVertex);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuD3d11Gfx::drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount)
{
	if ( mDynamicVBActive )
		setVertexBuffer(mpCurVB);

	if ( mDynamicIBActive )
		setIndexBuffer(mpCurIB);

	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);

	mpDeviceContext->IASetPrimitiveTopology(VuD3d11GfxTypes::convert(primitiveType));
	mpDeviceContext->DrawIndexed(vertexCount, startIndex, 0);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuD3d11Gfx::drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer)
{
	VuD3d11VertexBuffer *pD3d11VertexBuffer = static_cast<VuD3d11VertexBuffer *>(pVertexColorBuffer);

	VUUINT stride = 4;
	VUUINT offset = 0;
	mpDeviceContext->IASetVertexBuffers(1, 1, &pD3d11VertexBuffer->mpD3dVertexBuffer, &stride, &offset);

	drawIndexedPrimitive(primitiveType, minIndex, numVerts, startIndex, primitiveCount);
}

//*****************************************************************************
void VuD3d11Gfx::drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData)
{
	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);

#ifndef VURETAIL
	if ( mDynamicVBOffset + vertexCount*mCurVertexStride > DYNAMIC_VB_SIZE )
	{
		VUPRINTF("Dynamic VB overflow!");
		return;
	}
#endif

	// write to dynamic vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	mpDeviceContext->Map(mpDynamicVB, 0, mDynamicVBOffset == 0 ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
	VU_MEMCPY((VUBYTE *)mappedResource.pData + mDynamicVBOffset, DYNAMIC_VB_SIZE - mDynamicVBOffset, pVertexData, vertexCount*mCurVertexStride);
	mpDeviceContext->Unmap(mpDynamicVB, 0);

	// draw
	mpDeviceContext->IASetVertexBuffers(0, 1, &mpDynamicVB, &mCurVertexStride, &mDynamicVBOffset);
	mpDeviceContext->IASetPrimitiveTopology(VuD3d11GfxTypes::convert(primitiveType));
	mpDeviceContext->Draw(vertexCount, 0);

	// info
	mDynamicVBOffset += vertexCount*mCurVertexStride;
	mDynamicVBActive = true;
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuD3d11Gfx::drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData)
{
	updateState();

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);

#ifndef VURETAIL
	if ( mDynamicVBOffset + numVerts*mCurVertexStride > DYNAMIC_VB_SIZE )
	{
		VUPRINTF("Dynamic VB overflow!");
		return;
	}
	if ( mDynamicIBOffset + vertexCount*2 > DYNAMIC_IB_SIZE )
	{
		VUPRINTF("Dynamic IB overflow!");
		return;
	}
#endif

	// write to dynamic vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	mpDeviceContext->Map(mpDynamicVB, 0, mDynamicVBOffset == 0 ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
	VU_MEMCPY((VUBYTE *)mappedResource.pData + mDynamicVBOffset, DYNAMIC_VB_SIZE - mDynamicVBOffset, pVertexData, numVerts*mCurVertexStride);
	mpDeviceContext->Unmap(mpDynamicVB, 0);

	// write to dynamic index buffer
	mpDeviceContext->Map(mpDynamicIB, 0, mDynamicIBOffset == 0 ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
	VU_MEMCPY((VUBYTE *)mappedResource.pData + mDynamicIBOffset, DYNAMIC_IB_SIZE - mDynamicIBOffset, pIndexData, vertexCount*2);
	mpDeviceContext->Unmap(mpDynamicIB, 0);

	// draw
	mpDeviceContext->IASetVertexBuffers(0, 1, &mpDynamicVB, &mCurVertexStride, &mDynamicVBOffset);
	mpDeviceContext->IASetIndexBuffer(mpDynamicIB, DXGI_FORMAT_R16_UINT, mDynamicIBOffset);
	mpDeviceContext->IASetPrimitiveTopology(VuD3d11GfxTypes::convert(primitiveType));
	mpDeviceContext->DrawIndexed(vertexCount, 0, 0);

	// info
	mDynamicVBOffset += numVerts*mCurVertexStride;
	mDynamicIBOffset += vertexCount*2;
	mDynamicVBActive = true;
	mDynamicIBActive = true;
	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuD3d11Gfx::updateState()
{
	if ( mRasterizerStateDirty )
	{
		// search existing states for match
		ID3D11RasterizerState *pRasterizerState = VUNULL;
		for ( RasterizerStates::iterator iter = mRasterizerStates.begin(); iter != mRasterizerStates.end(); iter++ )
		{
			if ( iter->mCullMode == mCullMode && iter->mScissorTestEnable == mScissorTestEnable )
			{
				pRasterizerState = iter->mpD3dRasterizerState;
				break;
			}
		}

		if ( pRasterizerState == VUNULL )
		{
			CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
			desc.CullMode = VuD3d11GfxTypes::convert(mCullMode);
			desc.FrontCounterClockwise = TRUE;
			desc.ScissorEnable = mScissorTestEnable;

			ID3D11RasterizerState *pRasterizerState = VUNULL;
			D3DCALL(mpDevice->CreateRasterizerState(&desc, &pRasterizerState));

			VuRasterizerState rasterizerState;
			rasterizerState.mCullMode = mCullMode;
			rasterizerState.mScissorTestEnable = mScissorTestEnable;
			rasterizerState.mpD3dRasterizerState = pRasterizerState;
			mRasterizerStates.push_back(rasterizerState);
		}

		mpDeviceContext->RSSetState(pRasterizerState);

		mRasterizerStateDirty = false;
	}

	if ( mpCurShaderProgram )
	{
		VuD3d11Shader *pVertexShader = mpCurShaderProgram->mapShaders[VuShaderProgram::VERTEX_SHADER];
		for ( VUUINT32 i = 0; i < pVertexShader->mConstantBufferCount; i++ )
		{
			if ( pVertexShader->mConstantBufferDirtyBits & (1<<i) )
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				D3DCALL(mpDeviceContext->Map(pVertexShader->mapD3d11ConstantBuffers[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				VU_MEMCPY(mappedResource.pData, pVertexShader->mpConstantShadowBuffers[i].mSize, pVertexShader->mpConstantShadowBuffers[i].mpData, pVertexShader->mpConstantShadowBuffers[i].mSize);
				mpDeviceContext->Unmap(pVertexShader->mapD3d11ConstantBuffers[i], 0);
			}
		}
		pVertexShader->mConstantBufferDirtyBits = 0;

		VuD3d11Shader *pPixelShader = mpCurShaderProgram->mapShaders[VuShaderProgram::PIXEL_SHADER];
		for ( VUUINT32 i = 0; i < pPixelShader->mConstantBufferCount; i++ )
		{
			if ( pPixelShader->mConstantBufferDirtyBits & (1<<i) )
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				D3DCALL(mpDeviceContext->Map(pPixelShader->mapD3d11ConstantBuffers[i], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				VU_MEMCPY(mappedResource.pData, pPixelShader->mpConstantShadowBuffers[i].mSize, pPixelShader->mpConstantShadowBuffers[i].mpData, pPixelShader->mpConstantShadowBuffers[i].mSize);
				mpDeviceContext->Unmap(pPixelShader->mapD3d11ConstantBuffers[i], 0);
			}
		}
		pPixelShader->mConstantBufferDirtyBits = 0;
	}
}

//*****************************************************************************
HRESULT VuD3d11Gfx::createDepthStencilView(int width, int height, DXGI_FORMAT format, ID3D11DepthStencilView **ppDepthStencilView)
{
	HRESULT hres;

	// create depth texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	ID3D11Texture2D *pDepthTexture;
	hres = VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, VUNULL, &pDepthTexture);
	if ( hres != S_OK )
		return hres;

	// create depth view
	hres = VuD3d11Gfx::IF()->getD3dDevice()->CreateDepthStencilView(pDepthTexture, NULL, ppDepthStencilView);
	if ( hres != S_OK )
	{
		pDepthTexture->Release();
		return hres;
	}

	// clean up
	pDepthTexture->Release();

	return S_OK;
}

//*****************************************************************************
void VuD3d11Gfx::resetStats()
{
	VuGfx::resetStats();

	mMaxDynamicVBSize = VuMax(mMaxDynamicVBSize, mCurDynamicVBSize);
	mMaxDynamicIBSize = VuMax(mMaxDynamicIBSize, mCurDynamicIBSize);

	mCurDynamicVBSize = 0;
	mCurDynamicIBSize = 0;
}

//*****************************************************************************
void VuD3d11Gfx::printStats()
{
	VuGfx::printStats();

	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "Gfx" )
			{
				pPage->printf("DVB: size-%dK max-%dK cur-%dK\n", DYNAMIC_VB_SIZE/1024, mMaxDynamicVBSize/1024, mCurDynamicVBSize/1024);
				pPage->printf("DIB: size-%dK max-%dK cur-%dK\n", DYNAMIC_IB_SIZE/1024, mMaxDynamicIBSize/1024, mCurDynamicIBSize/1024);
			}
		}
	}
}
