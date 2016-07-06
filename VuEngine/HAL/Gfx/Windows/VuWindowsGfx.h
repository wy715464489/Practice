//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 D3d11 interface class for Gfx.
//
//*****************************************************************************

#pragma once

#define D3DXFX_LARGEADDRESS_HANDLE

#include <d3d11_1.h>
#include <dxgi1_3.h>
//#include <d3dx11.h>

#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"
#include "VuEngine/Math/VuRect.h"


class VuGfxDisplay;


class VuWindowsD3d11Gfx : public VuD3d11Gfx
{
protected:
	virtual bool init(VUHANDLE hWindow, VUHANDLE hDevice);
	virtual void release();

public:
	VuWindowsD3d11Gfx();
	~VuWindowsD3d11Gfx();

	// platform-specific functionality
	static VuWindowsD3d11Gfx *IF() { return static_cast<VuWindowsD3d11Gfx *>(VuGfx::IF()); }

	// setters
	void				setCompositionScale(float scaleX, float scaleY);

	// getters
	IDXGIDevice3		*getDxgiDevice() { return mpDxgiDevice; }
	IDXGISwapChain1		*getSwapChain() { return mpSwapChain; }

	// cross-platform functionality

	// thread ownership (not required)
	virtual void		acquireThreadOwnership();
	virtual void		releaseThreadOwnership();

	// set focus window (not required)
	virtual void		setFocusWindow(VUHANDLE hWndFocus);

	// display size
	virtual void		resize(VUHANDLE hDisplay, int width, int height);
	virtual void		getDisplaySize(VUHANDLE hDisplay, int &width, int &height);

	// render targets
	virtual void		setRenderTarget(const VuSetRenderTargetParams &params);

	// begin/end
	virtual bool		beginScene(VUHANDLE hDisplay);
	virtual bool		endScene(VUHANDLE hDisplay);

	virtual void		syncPostDraw();

private:
	VUHANDLE				mCurrentRenderThread;
	IDXGIDevice3			*mpDxgiDevice;
	IDXGISwapChain1			*mpSwapChain;
	ID3D11RenderTargetView	*mpRenderTargetView;
	ID3D11DepthStencilView	*mpDepthStencilView;
	int						mDisplayWidth;
	int						mDisplayHeight;
};
