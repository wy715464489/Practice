//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 D3d11 RenderTarget interface class.
// 
//*****************************************************************************

#include "VuD3d11RenderTarget.h"
#include "VuD3d11Texture.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"


//*****************************************************************************
VuD3d11RenderTarget::VuD3d11RenderTarget(int width, int height):
	VuRenderTarget(width, height)
{
}

//*****************************************************************************
VuD3d11RenderTarget::~VuD3d11RenderTarget()
{
	mpD3d11DepthStencilView->Release();
	mpD3d11ColorView->Release();
	mpDepthTexture->removeRef();
	mpColorTexture->removeRef();
}

//*****************************************************************************
VuTexture *VuD3d11RenderTarget::getColorTexture()
{
	return mpColorTexture; 
}

//*****************************************************************************
void VuD3d11RenderTarget::readPixels(VuArray<VUBYTE> &rgb)
{
	int width = getWidth();
	int height = getHeight();

	// crate texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.BindFlags = 0;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDesc.MiscFlags = 0;
	
	ID3D11Texture2D *pTexture = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, NULL, &pTexture));

	// copy to texture
	VuD3d11Gfx::IF()->getD3dDeviceContext()->CopyResource(pTexture, mpColorTexture->mpD3dTexture);

	// map resource
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDeviceContext()->Map(pTexture, 0, D3D11_MAP_READ, 0, &mappedResource));

	// read pixels
	rgb.resize(width*height*3);
	VUBYTE *pDst = static_cast<VUBYTE *>(&rgb[0]);
	for ( int y = 0; y < height; y++ )
	{
		VUBYTE *pSrc = static_cast<VUBYTE *>(mappedResource.pData) + y*mappedResource.RowPitch;
		for ( int x = 0; x < width; x++ )
		{
			pDst[0] = pSrc[0];
			pDst[1] = pSrc[1];
			pDst[2] = pSrc[2];
			pSrc += 4;
			pDst += 3;
		}
	}

	// clean up
	VuD3d11Gfx::IF()->getD3dDeviceContext()->Unmap(pTexture, 0);
	pTexture->Release();
}

//*****************************************************************************
VuD3d11RenderTarget *VuD3d11RenderTarget::create(int width, int height)
{
	// determine format
	DXGI_FORMAT d3dDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT d3dFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// create depth texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_LINEAR;
	state.mMinFilter = VUGFX_TEXF_LINEAR;
	state.mMipFilter = VUGFX_TEXF_NONE;
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

	// create render target
	VuD3d11RenderTarget *pRenderTarget = new VuD3d11RenderTarget(width, height);
	pRenderTarget->mpDepthTexture = pDepthTexture;
	pRenderTarget->mpColorTexture = pColorTexture;
	pRenderTarget->mpD3d11DepthStencilView = pD3d11DepthStencilView;
	pRenderTarget->mpD3d11ColorView = pD3d11ColorView;

	return pRenderTarget;
}
