//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the texture interface class.
//
//*****************************************************************************

#include "VuD3d11Texture.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


//*****************************************************************************
VuD3d11Texture::VuD3d11Texture(int width, int height, int levelCount, const VuTextureState &state) :
	VuTexture(width, height, levelCount),
	mpD3dTexture(VUNULL),
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
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateSamplerState(&samplerDesc, &mpD3dSamplerState));
}

//*****************************************************************************
VuD3d11Texture::~VuD3d11Texture()
{
	D3DRELEASE(mpD3dResourceView);
	D3DRELEASE(mpD3dTexture);
	D3DRELEASE(mpD3dSamplerState);
}

//*****************************************************************************
void VuD3d11Texture::setData(int level, const void *pData, int size)
{
	int levelWidth = VuMax(mWidth >> level, 1);
	int levelHeight = VuMax(mHeight >> level, 1);

	VUASSERT(level < mLevelCount, "VuD3d11Texture::setData() invalid level");

	int bytesPerPixel = 0;
	if ( mD3dFormat == DXGI_FORMAT_R8_UNORM )
		bytesPerPixel = 1;
	else if ( mD3dFormat == DXGI_FORMAT_R8G8_SNORM )
		bytesPerPixel = 2;
	else if ( mD3dFormat == DXGI_FORMAT_B5G6R5_UNORM )
		bytesPerPixel = 2;
	else if ( mD3dFormat == DXGI_FORMAT_R8G8B8A8_UNORM )
		bytesPerPixel = 4;
	else if ( mD3dFormat == DXGI_FORMAT_R16G16B16A16_SNORM )
		bytesPerPixel = 8;
	else
		VUASSERT(0, "VuD3d11Texture::setData() unsupported texture format");

	int rowPitch = levelWidth*bytesPerPixel;
	int depthPitch = levelHeight*rowPitch;

	// make sure data size is what we expect
	VUASSERT(size == depthPitch, "VuD3d11Texture::setData() unexpected data size");

	UINT subresource = D3D11CalcSubresource(level, 0, mLevelCount);
	VuD3d11Gfx::IF()->getD3dDeviceContext()->UpdateSubresource(mpD3dTexture, subresource, NULL, pData, rowPitch, depthPitch);
}

//*****************************************************************************
bool VuD3d11Texture::reload(VuBinaryDataReader &reader, int skipLevels)
{
	VuD3d11Texture *pNewTexture = (VuD3d11Texture *)VuGfx::IF()->loadTexture(reader, skipLevels);
	if ( pNewTexture == VUNULL )
		return false;

	ULONG test0 = mpD3dTexture->Release();
	test0;
	
	*this = *pNewTexture;
	
	ULONG test1 = mpD3dTexture->AddRef();
	test1;

	pNewTexture->removeRef();
		
	return true;
}

//*****************************************************************************
VuD3d11Texture *VuD3d11Texture::load(VuBinaryDataReader &reader, int skipLevels)
{
	VuTextureState state;
	state.deserialize(reader);

	int width, height, levelCount;
	reader.readValue(width);
	reader.readValue(height);
	reader.readValue(levelCount);

	skipLevels = (levelCount > 4) ? skipLevels : 0;
	if ( skipLevels )
	{
		width = VuMax(width>>skipLevels, 1);
		height = VuMax(height>>skipLevels, 1);
		levelCount -= skipLevels;
	}

	DXGI_FORMAT d3dFormat;
	reader.readValue(d3dFormat);

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = levelCount;
	textureDesc.ArraySize = 1;
	textureDesc.Format = d3dFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	ID3D11Texture2D *pD3dTexture = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, NULL, &pD3dTexture));

	ID3D11ShaderResourceView *pResourceView = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateShaderResourceView(pD3dTexture, NULL, &pResourceView));

	VuTextureData textureData;
	textureData.load(reader, skipLevels);

	for ( int iLevel = 0; iLevel < levelCount; iLevel++ )
	{
		UINT subresource = D3D11CalcSubresource(iLevel, 0, levelCount);
		const void *pData = textureData.getLevelData(iLevel);
		int rowPitch = textureData.getLevelPitch(iLevel);
		int depthPitch = textureData.getLevelSize(iLevel);

		VuD3d11Gfx::IF()->getD3dDeviceContext()->UpdateSubresource(pD3dTexture, subresource, NULL, pData, rowPitch, depthPitch);
	}

	VuD3d11Texture *pTexture = new VuD3d11Texture(width, height, levelCount, state);

	pTexture->mD3dFormat = d3dFormat;
	pTexture->mpD3dTexture = pD3dTexture;
	pTexture->mpD3dResourceView = pResourceView;

	return pTexture;
}

//*****************************************************************************
VuD3d11Texture *VuD3d11Texture::create(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	bool createMipMaps = (state.mMipFilter != VUGFX_TEXF_NONE);
	int levelCount = createMipMaps ? VuHighBit(VuMax(width, height)) + 1 : 1;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = levelCount;
	textureDesc.ArraySize = 1;
	textureDesc.Format = VuD3d11GfxTypes::convert(format);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	ID3D11Texture2D *pD3dTexture = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateTexture2D(&textureDesc, VUNULL, &pD3dTexture));

	ID3D11ShaderResourceView *pResourceView = NULL;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateShaderResourceView(pD3dTexture, NULL, &pResourceView));

	VuD3d11Texture *pTexture = new VuD3d11Texture(width, height, levelCount, state);
	pTexture->mD3dFormat = textureDesc.Format;
	pTexture->mpD3dTexture = pD3dTexture;
	pTexture->mpD3dResourceView = pResourceView;

	return pTexture;
}