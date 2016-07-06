//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Gfx library.
//
//  Uses D3d11 on Win32
// 
//*****************************************************************************

#include "VuWin32Gfx.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Texture.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11CubeTexture.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11IndexBuffer.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11VertexBuffer.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11RenderTarget.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11ShadowRenderTarget.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/Json/VuJsonContainer.h"


// libs

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGfx, VuWin32D3d11Gfx);


// constants
#define SWAP_CHAIN_BUFFER_COUNT 1


class VuGfxDisplay
{
public:
	VuGfxDisplay() : mWidth(0), mHeight(0), mpSwapChain(0), mpRenderTargetView(0), mpDepthStencilView(0)
	{
	}
	~VuGfxDisplay()
	{
		D3DRELEASE(mpDepthStencilView);
		D3DRELEASE(mpRenderTargetView);
		D3DRELEASE(mpSwapChain);
	}
	int						mWidth;
	int						mHeight;
	IDXGISwapChain			*mpSwapChain;
	ID3D11RenderTargetView	*mpRenderTargetView;
	ID3D11DepthStencilView	*mpDepthStencilView;
};



//*****************************************************************************
VuWin32D3d11Gfx::VuWin32D3d11Gfx() :
	mCurrentRenderThread(VUNULL),
	mpIDXGIFactory(VUNULL),
	mpSwapChain(VUNULL),
	mpRenderTargetView(VUNULL),
	mpDepthStencilView(VUNULL),
	mDisplayWidth(0),
	mDisplayHeight(0),
	mpCurDisplay(VUNULL)
{
}

//*****************************************************************************
VuWin32D3d11Gfx::~VuWin32D3d11Gfx()
{
}

//*****************************************************************************
bool VuWin32D3d11Gfx::init(VUHANDLE hWindow, VUHANDLE hDevice)
{
	// create device
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef VUDEBUG
	//flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	if (hWindow)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		memset(&swapChainDesc, 0, sizeof(swapChainDesc));

		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
		swapChainDesc.OutputWindow = (HWND)hWindow;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		D3D_FEATURE_LEVEL featureLevel;
		HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapChainDesc, &mpSwapChain, &mpDevice, &featureLevel, &mpDeviceContext);
		if (result != S_OK)
		{
			VUPRINTF("Error: D3D11CreateDeviceAndSwapChain() failed with HRESULT %ld (%0lx) - This may be D3D_FEATURE_LEVEL_11_1 incompatibility, so let's try 11_0\n", 
						result, result);

			// As a fallback, ignore the first FEATURE_LEVEL and start with 11_0 instead of 11_1 in cases that we're running on systems that flat-out don't know about 11_1.
			// This is some bad code in DirectX that doesn't do graceful fallbacks while looking at feature levels. Hopefully this has been fixed for future versions of 
			// DirectX
			result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, &featureLevels[1], ARRAYSIZE(featureLevels) - 1, D3D11_SDK_VERSION, &swapChainDesc, &mpSwapChain, &mpDevice, &featureLevel, &mpDeviceContext);
			if (result != S_OK)
			{
				VUPRINTF("Error: D3D11CreateDeviceAndSwapChain() failed with HRESULT %ld (%0lx) - This was the fallback, so something even badder has happened.\n",
					result, result);

				return false;
			}
		}

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

		setViewport(VuRect(0, 0, 1, 1));
	}
	else
	{
		D3D_FEATURE_LEVEL featureLevel;
		if (D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &mpDevice, &featureLevel, &mpDeviceContext) != S_OK)
		{
			VUPRINTF("Error: D3D11CreateDevice() failed\n");
			return false;
		}
	}

	mCurrentRenderThread = GetCurrentThread();

	// get interface to DXGI Factory
	IDXGIDevice2 *pDXGIDevice;
	IDXGIAdapter *pDXGIAdapter;

	D3DCALL(mpDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice));
	D3DCALL(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter));
	D3DCALL(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&mpIDXGIFactory));
	D3DRELEASE(pDXGIAdapter);
	D3DRELEASE(pDXGIDevice);

	// don't allow alt-enter (we handle it ourselves)
	D3DCALL(mpIDXGIFactory->MakeWindowAssociation((HWND)hWindow, DXGI_MWA_NO_ALT_ENTER));

	if (!VuD3d11Gfx::init())
		return false;

	return true;
}

//*****************************************************************************
void VuWin32D3d11Gfx::release()
{
	for (Displays::iterator iter = mDisplays.begin(); iter != mDisplays.end(); iter++)
		delete *iter;
	mDisplays.clear();

	VuD3d11Gfx::release();

	mpDeviceContext->ClearState();
	mpDeviceContext->Flush();

	mpDeviceContext->OMSetRenderTargets(0, NULL, NULL);

	if ( mpSwapChain )
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
IDXGISwapChain *VuWin32D3d11Gfx::getSwapChain(VUHANDLE hDisplay)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	if (pDisplay)
		return pDisplay->mpSwapChain;

	return mpSwapChain;
}

//*****************************************************************************
void VuWin32D3d11Gfx::acquireThreadOwnership()
{
	VUASSERT(mCurrentRenderThread == VUNULL, "VuWin32D3d11Gfx::acquireThreadOwnership() thread already owned");
	mCurrentRenderThread = GetCurrentThread();
}

//*****************************************************************************
void VuWin32D3d11Gfx::releaseThreadOwnership()
{
	VUASSERT(mCurrentRenderThread == GetCurrentThread(), "VuWin32D3d11Gfx::releaseThreadOwnership() thread not owned by calling thread");
	mCurrentRenderThread = VUNULL;
}

//*****************************************************************************
void VuWin32D3d11Gfx::setFocusWindow(VUHANDLE hWndFocus)
{
	//mD3dPresentParams.hDeviceWindow = (HWND)hWndFocus;
}

//*****************************************************************************
void VuWin32D3d11Gfx::resize(VUHANDLE hDisplay, int width, int height)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	if (width > 0 && height > 0)
	{
		if (pDisplay)
		{
			if (pDisplay->mWidth != width || pDisplay->mHeight != height)
			{
				mpDeviceContext->OMSetRenderTargets(0, NULL, NULL);
				D3DRELEASE(pDisplay->mpDepthStencilView);
				D3DRELEASE(pDisplay->mpRenderTargetView);

				D3DCALL(pDisplay->mpSwapChain->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

				ID3D11Texture2D *pBackBufferTexture;
				D3DCALL(pDisplay->mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBufferTexture));
				D3DCALL(mpDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &pDisplay->mpRenderTargetView));
				pBackBufferTexture->Release();

				D3DCALL(createDepthStencilView(width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, &pDisplay->mpDepthStencilView));

				pDisplay->mWidth = width;
				pDisplay->mHeight = height;

				mCurRenderTargetWidth = width;
				mCurRenderTargetHeight = height;

				setViewport(VuRect(0, 0, 1, 1));
			}
		}
		else
		{
			if (mDisplayWidth != width || mDisplayHeight != height)
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
}

//*****************************************************************************
void VuWin32D3d11Gfx::getDisplaySize(VUHANDLE hDisplay, int &width, int &height)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	if (pDisplay)
	{
		width = pDisplay->mWidth;
		height = pDisplay->mHeight;
	}
	else
	{
		width = mDisplayWidth;
		height = mDisplayHeight;
	}
}

//*****************************************************************************
void VuWin32D3d11Gfx::setRenderTarget(const VuSetRenderTargetParams &params)
{
	ID3D11RenderTargetView *pRenderTargetView = NULL;
	ID3D11DepthStencilView *pDepthStencilView = NULL;

	if (params.mpRenderTarget)
	{
		VuD3d11RenderTarget *pD3d11RenderTarget = (VuD3d11RenderTarget *)params.mpRenderTarget;

		pRenderTargetView = pD3d11RenderTarget->mpD3d11ColorView;
		pDepthStencilView = pD3d11RenderTarget->mpD3d11DepthStencilView;

		mCurRenderTargetWidth = pD3d11RenderTarget->getWidth();
		mCurRenderTargetHeight = pD3d11RenderTarget->getHeight();

	}
	else if (mpCurDisplay)
	{
		pRenderTargetView = mpCurDisplay->mpRenderTargetView;
		pDepthStencilView = mpCurDisplay->mpDepthStencilView;

		mCurRenderTargetWidth = mpCurDisplay->mWidth;
		mCurRenderTargetHeight = mpCurDisplay->mHeight;
	}
	else
	{
		pRenderTargetView = mpRenderTargetView;
		pDepthStencilView = mpDepthStencilView;

		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	mpDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	setViewport(VuRect(0, 0, 1, 1));

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

//*****************************************************************************
VUHANDLE VuWin32D3d11Gfx::createDisplay(VUHANDLE hWnd)
{
	VuGfxDisplay *pDisplay = new VuGfxDisplay;

	// get window width/height
	RECT rect;
	if (!GetClientRect((HWND)hWnd, &rect))
		return false;
	pDisplay->mWidth = VuMax(rect.right, 32);
	pDisplay->mHeight = VuMax(rect.bottom, 32);

	// create swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));

	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.OutputWindow = (HWND)hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	D3DCALL(mpIDXGIFactory->CreateSwapChain(mpDevice, &swapChainDesc, &pDisplay->mpSwapChain));

	// create render target view
	ID3D11Texture2D *pBackBufferTexture;
	D3DCALL(pDisplay->mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBufferTexture));
	D3DCALL(mpDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &pDisplay->mpRenderTargetView));
	D3D11_TEXTURE2D_DESC backBufferDesc;
	pBackBufferTexture->GetDesc(&backBufferDesc);
	pBackBufferTexture->Release();

	// keep track
	mDisplays.push_back(pDisplay);

	mCurRenderTargetWidth = pDisplay->mWidth;
	mCurRenderTargetHeight = pDisplay->mHeight;

	setViewport(VuRect(0, 0, 1, 1));

	return pDisplay;
}

//*****************************************************************************
void VuWin32D3d11Gfx::releaseDisplay(VUHANDLE hDisplay)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	mDisplays.remove(pDisplay);
	delete pDisplay;
}

//*****************************************************************************
bool VuWin32D3d11Gfx::beginScene(VUHANDLE hDisplay)
{
	if (!VuD3d11Gfx::beginScene(hDisplay))
		return false;

	mpCurDisplay = (VuGfxDisplay *)hDisplay;

	if (mpCurDisplay)
	{
		mpDeviceContext->OMSetRenderTargets(1, &mpCurDisplay->mpRenderTargetView, mpCurDisplay->mpDepthStencilView);

		mCurRenderTargetWidth = mpCurDisplay->mWidth;
		mCurRenderTargetHeight = mpCurDisplay->mHeight;
	}
	else
	{
		mpDeviceContext->OMSetRenderTargets(1, &mpRenderTargetView, mpDepthStencilView);

		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	setViewport(VuRect(0, 0, 1, 1));

	return true;
}

//*****************************************************************************
bool VuWin32D3d11Gfx::endScene(VUHANDLE hDisplay)
{
	VUASSERT(hDisplay == mpCurDisplay, "VuGfx beginScene/endScene mismatch");

	if (!VuD3d11Gfx::endScene(hDisplay))
		return false;

	return true;
}

//*****************************************************************************
void VuWin32D3d11Gfx::syncPostDraw()
{
	// flip
	HRESULT hRes;
	if (mpCurDisplay)
	{
		hRes = mpCurDisplay->mpSwapChain->Present(mFlipInterval, 0);
	}
	else
	{
		hRes = mpSwapChain->Present(mFlipInterval, 0);
	}

	// If the device was removed either by a disconnect or a driver upgrade, we 
	// must completely reinitialize the renderer.
	if (hRes == DXGI_ERROR_DEVICE_REMOVED || hRes == DXGI_ERROR_DEVICE_RESET)
	{
	}

	mpCurDisplay = VUNULL;
}