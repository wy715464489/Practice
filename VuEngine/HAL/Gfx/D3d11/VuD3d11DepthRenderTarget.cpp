//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 DepthRenderTarget interface class.
// 
//*****************************************************************************

#include "VuD3d11DepthRenderTarget.h"
#include "VuD3d11Texture.h"
#include "VuD3d11Gfx.h"


//*****************************************************************************
VuD3d11DepthRenderTarget::VuD3d11DepthRenderTarget(int width, int height) :
	VuDepthRenderTarget(width, height)
{
}

//*****************************************************************************
VuD3d11DepthRenderTarget::~VuD3d11DepthRenderTarget()
{
	mpD3d11DepthStencilView->Release();
	mpD3d11ColorView->Release();
	mpDepthTexture->removeRef();
	mpColorTexture->removeRef();
}

//*****************************************************************************
VuTexture *VuD3d11DepthRenderTarget::getTexture()
{
	return mpColorTexture;
}

//*****************************************************************************
VuD3d11DepthRenderTarget *VuD3d11DepthRenderTarget::create(int width, int height)
{
	// determine format
	DXGI_FORMAT d3dDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT d3dFormat = DXGI_FORMAT_R32_FLOAT;

	// texture state
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_LINEAR;
	state.mMinFilter = VUGFX_TEXF_LINEAR;
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

	// create depth render target
	VuD3d11DepthRenderTarget *pDepthRenderTarget = new VuD3d11DepthRenderTarget(width, height);
	pDepthRenderTarget->mpDepthTexture = pDepthTexture;
	pDepthRenderTarget->mpColorTexture = pColorTexture;
	pDepthRenderTarget->mpD3d11DepthStencilView = pD3d11DepthStencilView;
	pDepthRenderTarget->mpD3d11ColorView = pD3d11ColorView;

	return pDepthRenderTarget;
}
