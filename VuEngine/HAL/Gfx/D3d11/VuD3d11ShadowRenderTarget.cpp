//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 ShadowRenderTarget interface class.
// 
//*****************************************************************************

#include "VuD3d11ShadowRenderTarget.h"
#include "VuD3d11Texture.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"


//*****************************************************************************
VuD3d11ShadowRenderTarget::VuD3d11ShadowRenderTarget(int width, int height, int count):
	VuShadowRenderTarget(width, height, count),
	mColorBuffers(count)
{
}

//*****************************************************************************
VuD3d11ShadowRenderTarget::~VuD3d11ShadowRenderTarget()
{
	mpD3d11DepthStencilView->Release();
	mpDepthTexture->removeRef();

	for ( int i = 0; i < mColorBuffers.size(); i++ )
	{
		mColorBuffers[i].mpD3d11View->Release();
		mColorBuffers[i].mpTexture->removeRef();
	}
}

//*****************************************************************************
VuTexture *VuD3d11ShadowRenderTarget::getColorTexture(int layer)
{
	return mColorBuffers[layer].mpTexture; 
}

//*****************************************************************************
VuD3d11ShadowRenderTarget *VuD3d11ShadowRenderTarget::create(int width, int height, int count)
{
	// determine format
	DXGI_FORMAT d3dDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT d3dFormat = DXGI_FORMAT_R32_FLOAT;

	// texture state
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_POINT;
	state.mMinFilter = VUGFX_TEXF_POINT;
	state.mMipFilter = VUGFX_TEXF_NONE;

	// create depth texture
	VuD3d11Texture *pDepthTexture = new VuD3d11Texture(width, height, 1, state);
	pDepthTexture->mD3dFormat = d3dDepthFormat;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = d3dDepthFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, VUNULL, &pDepthTexture->mpD3dTexture));

	// create depth view
	ID3D11DepthStencilView *pD3d11DepthStencilView = VUNULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateDepthStencilView(pDepthTexture->mpD3dTexture, NULL, &pD3d11DepthStencilView));

	// create shadow render target
	VuD3d11ShadowRenderTarget *pShadowRenderTarget = new VuD3d11ShadowRenderTarget(width, height, count);
	pShadowRenderTarget->mpDepthTexture = pDepthTexture;
	pShadowRenderTarget->mpD3d11DepthStencilView = pD3d11DepthStencilView;

	// create color textures
	for ( int i = 0; i < count; i++ )
	{
		// create color texture
		VuD3d11Texture *pColorTexture = new VuD3d11Texture(width, height, 1, state);
		pColorTexture->mD3dFormat = d3dFormat;

		textureDesc.Format = d3dFormat;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, VUNULL, &pColorTexture->mpD3dTexture));
		D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateShaderResourceView(pColorTexture->mpD3dTexture, NULL, &pColorTexture->mpD3dResourceView));

		// create color view
		ID3D11RenderTargetView *pD3d11ColorView = VUNULL;
		D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateRenderTargetView(pColorTexture->mpD3dTexture, NULL, &pD3d11ColorView));

		VuD3d11ShadowRenderTarget::ColorBuffer colorBuffer;
		colorBuffer.mpTexture = pColorTexture;
		colorBuffer.mpD3d11View = pD3d11ColorView;

		pShadowRenderTarget->mColorBuffers.push_back(colorBuffer);
	}

	return pShadowRenderTarget;
}
