//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the cube texture interface class.
// 
//*****************************************************************************

#include "VuD3d11CubeTexture.h"
#include "VuD3d11Gfx.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


//*****************************************************************************
VuD3d11CubeTexture::VuD3d11CubeTexture(int edgeLength, int levelCount, const VuTextureState &state) :
	VuCubeTexture(edgeLength, levelCount),
	mpD3dCubeTexture(VUNULL),
	mpD3dResourceView(VUNULL),
	mpD3dSamplerState(VUNULL)
{
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = VuD3d11GfxTypes::convert(state.mMagFilter, state.mMinFilter, state.mMipFilter);
	samplerDesc.AddressU = VuD3d11GfxTypes::convert(state.mAddressU);
	samplerDesc.AddressV = VuD3d11GfxTypes::convert(state.mAddressV);
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = (state.mMipFilter == VUGFX_TEXF_NONE) ? 0.0f : D3D11_FLOAT32_MAX;

	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateSamplerState(&samplerDesc, &mpD3dSamplerState));
}

//*****************************************************************************
VuD3d11CubeTexture::~VuD3d11CubeTexture()
{
	D3DRELEASE(mpD3dResourceView);
	D3DRELEASE(mpD3dCubeTexture);
	D3DRELEASE(mpD3dSamplerState);
}

//*****************************************************************************
bool VuD3d11CubeTexture::reload(VuBinaryDataReader &reader, int skipLevels)
{
	VuD3d11CubeTexture *pNewCubeTexture = (VuD3d11CubeTexture *)VuGfx::IF()->loadCubeTexture(reader, skipLevels);
	if ( pNewCubeTexture == VUNULL )
		return false;

	mpD3dCubeTexture->Release();
	*this = *pNewCubeTexture;
	mpD3dCubeTexture->AddRef();

	pNewCubeTexture->removeRef();

	return true;
}

//*****************************************************************************
VuD3d11CubeTexture *VuD3d11CubeTexture::load(VuBinaryDataReader &reader, int skipLevels)
{
	VuTextureState state;
	state.deserialize(reader);

	int edgeLength, levelCount;
	reader.readValue(edgeLength);
	reader.readValue(levelCount);

	skipLevels = (levelCount > 4) ? skipLevels : 0;
	if ( skipLevels )
	{
		edgeLength = VuMax(edgeLength>>skipLevels, 1);
		levelCount -= skipLevels;
	}
	
	// super-hack to fix Win8 D3D11 -> D3D9 API issue
	if ( VuD3d11Gfx::IF()->getD3dDevice()->GetFeatureLevel() < D3D_FEATURE_LEVEL_10_0 )
		levelCount = 1;

	DXGI_FORMAT d3dFormat;
	reader.readValue(d3dFormat);

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = edgeLength;
	textureDesc.Height = edgeLength;
	textureDesc.MipLevels = levelCount;
	textureDesc.ArraySize = 6;
	textureDesc.Format = d3dFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	
	ID3D11Texture2D *pD3dCubeTexture = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, NULL, &pD3dCubeTexture));

	ID3D11ShaderResourceView *pResourceView = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateShaderResourceView(pD3dCubeTexture, NULL, &pResourceView));

	VuTextureData textureData;
	for ( int iFace = 0; iFace < 6; iFace++ )
	{
		textureData.load(reader, skipLevels);

		for ( int iLevel = 0; iLevel < levelCount; iLevel++ )
		{
			UINT subresource = D3D11CalcSubresource(iLevel, iFace, levelCount);
			const void *pData = textureData.getLevelData(iLevel);
			int rowPitch = textureData.getLevelPitch(iLevel);
			int depthPitch = textureData.getLevelSize(iLevel);

			VuD3d11Gfx::IF()->getD3dDeviceContext()->UpdateSubresource(pD3dCubeTexture, subresource, NULL, pData, rowPitch, depthPitch);
		}
	}

	VuD3d11CubeTexture *pCubeTexture = new VuD3d11CubeTexture(edgeLength, levelCount, state);

	pCubeTexture->mpD3dCubeTexture = pD3dCubeTexture;
	pCubeTexture->mpD3dResourceView = pResourceView;

	return pCubeTexture;
}
