//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 D3d11 interface class for Gfx.
//
//*****************************************************************************

#pragma once

#define D3DXFX_LARGEADDRESS_HANDLE

#include <d3d11_x.h>

#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"
#include "VuEngine/Math/VuRect.h"


class VuGfxDisplay;


class VuXb1D3d11Gfx : public VuD3d11Gfx
{
protected:
	virtual bool init(VUHANDLE hWindow, VUHANDLE hDevice);
	virtual void release();

public:
	VuXb1D3d11Gfx();
	~VuXb1D3d11Gfx();

	// platform-specific functionality
	static VuXb1D3d11Gfx *IF() { return static_cast<VuXb1D3d11Gfx *>(VuGfx::IF()); }

	bool				isSuspended() { return mIsSuspended; }
	void				suspend();
	void				resume();

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

	// capabilities
	virtual bool		supportsSSAO() { return true; }
	virtual bool		supportsHBAO() { return true; }

private:
	VUHANDLE				mCurrentRenderThread;
	IDXGIFactory			*mpIDXGIFactory;
	IDXGISwapChain1			*mpSwapChain;
	ID3D11RenderTargetView	*mpRenderTargetView;
	ID3D11DepthStencilView	*mpDepthStencilView;
	int						mDisplayWidth;
	int						mDisplayHeight;
	bool					mIsSuspended;
};
