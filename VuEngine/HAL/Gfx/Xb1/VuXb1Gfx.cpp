//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Gfx library.
//
//  Uses D3d11 on Xb1
// 
//*****************************************************************************

#include "VuXb1Gfx.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Texture.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11CubeTexture.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11IndexBuffer.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11VertexBuffer.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11RenderTarget.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11DepthRenderTarget.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11ShadowRenderTarget.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/Json/VuJsonContainer.h"


// libs

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGfx, VuXb1D3d11Gfx);


// constants
#define SWAP_CHAIN_BUFFER_COUNT 2


//*****************************************************************************
VuXb1D3d11Gfx::VuXb1D3d11Gfx():
	mCurrentRenderThread(VUNULL),
	mpSwapChain(VUNULL),
	mpRenderTargetView(VUNULL),
	mpDepthStencilView(VUNULL),
	mDisplayWidth(0),
	mDisplayHeight(0),
	mIsSuspended(false)
{
}

//*****************************************************************************
VuXb1D3d11Gfx::~VuXb1D3d11Gfx()
{
}

//*****************************************************************************
bool VuXb1D3d11Gfx::init(VUHANDLE hWindow, VUHANDLE hDevice)
{
	// create device
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED;
	#ifdef VUDEBUG
//		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_0,
    };

	D3D_FEATURE_LEVEL featureLevel;
	if ( D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext) != S_OK )
		return false;

	// create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));

	swapChainDesc.Width = 1920;
	swapChainDesc.Height = 1080;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications

	IDXGIDevice1 *pDXGIDevice;
	IDXGIAdapter *pDXGIAdapter;
	IDXGIFactory2 *pIDXGIFactory;

	D3DCALL(mpDevice->QueryInterface(__uuidof(IDXGIDevice1), (void **)&pDXGIDevice));
	D3DCALL(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter));
	D3DCALL(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&pIDXGIFactory));

	//D3DCALL(pDXGIDevice->SetMaximumFrameLatency(1));
	D3DCALL(pIDXGIFactory->CreateSwapChainForCoreWindow(mpDevice, (IUnknown *)hWindow, &swapChainDesc, NULL, &mpSwapChain));

	pIDXGIFactory->Release();
	pDXGIAdapter->Release();
	pDXGIDevice->Release();

	// create render target view
	ID3D11Texture2D *pBackBufferTexture;
	D3DCALL(mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBufferTexture));
	D3DCALL(mpDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &mpRenderTargetView));
    D3D11_TEXTURE2D_DESC backBufferDesc;
    pBackBufferTexture->GetDesc(&backBufferDesc);
	pBackBufferTexture->Release();

	// create depth stencil view
	D3DCALL(createDepthStencilView(backBufferDesc.Width, backBufferDesc.Height, DXGI_FORMAT_D24_UNORM_S8_UINT, &mpDepthStencilView));

	// set render target
	mpDeviceContext->OMSetRenderTargets(1, &mpRenderTargetView, mpDepthStencilView);

	mDisplayWidth = backBufferDesc.Width;
	mDisplayHeight = backBufferDesc.Height;

	mCurRenderTargetWidth = backBufferDesc.Width;
	mCurRenderTargetHeight = backBufferDesc.Height;

	setViewport(VuRect(0,0,1,1));

	mCurrentRenderThread = GetCurrentThread();

	if ( !VuD3d11Gfx::init() )
		return false;

	return true;
}

//*****************************************************************************
void VuXb1D3d11Gfx::release()
{
	VuD3d11Gfx::release();

	mpDeviceContext->ClearState();
	mpDeviceContext->Flush();

	mpDeviceContext->OMSetRenderTargets(0, NULL, NULL);

	mpSwapChain->SetFullscreenState(FALSE, NULL);

	D3DRELEASE(mpDepthStencilView);
	D3DRELEASE(mpRenderTargetView);
	D3DRELEASE(mpSwapChain);
	D3DRELEASE(mpDeviceContext);
	D3DRELEASE(mpDevice);
	D3DRELEASE(mpIDXGIFactory);

	mCurrentRenderThread = VUNULL;
}

//*****************************************************************************
void VuXb1D3d11Gfx::suspend()
{
	ID3D11DeviceContextX* pXboxContext = static_cast<ID3D11DeviceContextX*>(mpDeviceContext);
	if (pXboxContext)
	{
		HRESULT hr = pXboxContext->Suspend(0);
		if (S_OK != hr)
		{
			VUPRINTF("ID3D11DeviceContextX::Suspend() returned %.X\n", hr);
		}

		mIsSuspended = true;
	}
}

//*****************************************************************************
void VuXb1D3d11Gfx::resume()
{
	ID3D11DeviceContextX* pXboxContext = static_cast<ID3D11DeviceContextX*>(mpDeviceContext);
	if (pXboxContext)
	{
		HRESULT hr = pXboxContext->Resume();
		if (S_OK != hr)
		{
			VUPRINTF("ID3D11DeviceContextX::Resume() returned %.X\n", hr);
		}

		mIsSuspended = false;
	}
}

//*****************************************************************************
void VuXb1D3d11Gfx::acquireThreadOwnership()
{
	VUASSERT(mCurrentRenderThread == VUNULL, "VuXb1D3d11Gfx::acquireThreadOwnership() thread already owned");
	mCurrentRenderThread = GetCurrentThread();
}

//*****************************************************************************
void VuXb1D3d11Gfx::releaseThreadOwnership()
{
	VUASSERT(mCurrentRenderThread == GetCurrentThread(), "VuXb1D3d11Gfx::releaseThreadOwnership() thread not owned by calling thread");
	mCurrentRenderThread = VUNULL;
}

//*****************************************************************************
void VuXb1D3d11Gfx::setFocusWindow(VUHANDLE hWndFocus)
{
	//mD3dPresentParams.hDeviceWindow = (HWND)hWndFocus;
}

//*****************************************************************************
void VuXb1D3d11Gfx::resize(VUHANDLE hDisplay, int width, int height)
{
	if ( width > 0 && height > 0 )
	{
		if ( mDisplayWidth != width || mDisplayHeight != height )
		{
			mpDeviceContext->OMSetRenderTargets(0, NULL, NULL);
			D3DRELEASE(mpDepthStencilView);
			D3DRELEASE(mpRenderTargetView);

			D3DCALL(mpSwapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

			ID3D11Texture2D *pBackBufferTexture;
			D3DCALL(mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBufferTexture));
			D3DCALL(mpDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &mpRenderTargetView));
			pBackBufferTexture->Release();

			D3DCALL(createDepthStencilView(width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, &mpDepthStencilView));

			mpDeviceContext->OMSetRenderTargets(1, &mpRenderTargetView, mpDepthStencilView);

			mDisplayWidth = width;
			mDisplayHeight = height;

			mCurRenderTargetWidth = width;
			mCurRenderTargetHeight = height;

			setViewport(VuRect(0, 0, 1, 1));
		}
	}
}

//*****************************************************************************
void VuXb1D3d11Gfx::getDisplaySize(VUHANDLE hDisplay, int &width, int &height)
{
	width = mDisplayWidth;
	height = mDisplayHeight;
}

//*****************************************************************************
bool VuXb1D3d11Gfx::beginScene(VUHANDLE hDisplay)
{
	if ( !VuD3d11Gfx::beginScene(hDisplay) )
		return false;

	mpDeviceContext->OMSetRenderTargets(1, &mpRenderTargetView, mpDepthStencilView);

	mCurRenderTargetWidth = mDisplayWidth;
	mCurRenderTargetHeight = mDisplayHeight;

	setViewport(VuRect(0, 0, 1, 1));
	clear(VUGFX_CLEAR_COLOR|VUGFX_CLEAR_DEPTH, VuColor(0,0,0), 1.0f);

	return true;
}

//*****************************************************************************
bool VuXb1D3d11Gfx::endScene(VUHANDLE hDisplay)
{
	if ( !VuD3d11Gfx::endScene(hDisplay) )
		return false;

	// flip
	HRESULT hr = mpSwapChain->Present(mFlipInterval, 0);

	// If the device was removed either by a disconnect or a driver upgrade, we 
	// must completely reinitialize the renderer.
	if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
	{
	}

	return true;
}

//*****************************************************************************

void VuXb1D3d11Gfx::setRenderTarget(const VuSetRenderTargetParams &params)
{
	ID3D11RenderTargetView *pRenderTargetView = NULL;
	ID3D11DepthStencilView *pDepthStencilView = NULL;

	if ( params.mpRenderTarget )
	{
		VuD3d11RenderTarget *pD3d11RenderTarget = (VuD3d11RenderTarget *)params.mpRenderTarget;

		pRenderTargetView = pD3d11RenderTarget->mpD3d11ColorView;
		pDepthStencilView = pD3d11RenderTarget->mpD3d11DepthStencilView;

		mCurRenderTargetWidth = pD3d11RenderTarget->getWidth();
		mCurRenderTargetHeight = pD3d11RenderTarget->getHeight();
	}
	else
	{
		pRenderTargetView = mpRenderTargetView;
		pDepthStencilView = mpDepthStencilView;

		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	mpDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	setViewport(VuRect(0,0,1,1));

	if ( pRenderTargetView && (params.mColorLoadAction == VuSetRenderTargetParams::LoadActionClear) )
	{
		VuVector4 vColor;
		params.mClearColor.toVector4(vColor);
		mpDeviceContext->ClearRenderTargetView(pRenderTargetView, &vColor.mX);
	}

	if ( pDepthStencilView && (params.mDepthLoadAction == VuSetRenderTargetParams::LoadActionClear) )
	{
		mpDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, params.mClearDepth, 0);
	}
}

