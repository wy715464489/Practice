//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 FxRenderTarget interface class.
// 
//*****************************************************************************

#include "VuD3d11FxRenderTarget.h"
#include "VuD3d11Texture.h"
#include "VuD3d11Gfx.h"


//*****************************************************************************
VuD3d11FxRenderTarget::VuD3d11FxRenderTarget(int width, int height, VuGfxFormat format) :
	VuFxRenderTarget(width, height, format)
{
}

//*****************************************************************************
VuD3d11FxRenderTarget::~VuD3d11FxRenderTarget()
{
	mpD3d11ColorView->Release();
	mpColorTexture->removeRef();
}

//*****************************************************************************
VuTexture *VuD3d11FxRenderTarget::getTexture()
{
	return mpColorTexture;
}

//*****************************************************************************
VuD3d11FxRenderTarget *VuD3d11FxRenderTarget::create(int width, int height, VuGfxFormat format)
{
	// determine format
	DXGI_FORMAT d3dFormat = VuD3d11GfxTypes::convert(format);

	// create color texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_LINEAR;
	state.mMinFilter = VUGFX_TEXF_LINEAR;
	state.mMipFilter = VUGFX_TEXF_NONE;
	VuD3d11Texture *pColorTexture = new VuD3d11Texture(width, height, 1, state);
	pColorTexture->mD3dFormat = d3dFormat;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = d3dFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, VUNULL, &pColorTexture->mpD3dTexture));
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateShaderResourceView(pColorTexture->mpD3dTexture, NULL, &pColorTexture->mpD3dResourceView));

	// create color view
	ID3D11RenderTargetView *pD3d11ColorView = VUNULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateRenderTargetView(pColorTexture->mpD3dTexture, NULL, &pD3d11ColorView));

	// create fx render target
	VuD3d11FxRenderTarget *pFxRenderTarget = new VuD3d11FxRenderTarget(width, height, format);
	pFxRenderTarget->mpColorTexture = pColorTexture;
	pFxRenderTarget->mpD3d11ColorView = pD3d11ColorView;

	return pFxRenderTarget;
}
