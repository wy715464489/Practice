//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the index buffer interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"


class VuD3d11IndexBuffer : public VuIndexBuffer
{
public:
	VuD3d11IndexBuffer(int count);
	~VuD3d11IndexBuffer();

	virtual void			resize(int newCount);
	virtual void			setData(const VUUINT16 *pData, int count);
	virtual const VUUINT16	*getShadowBuffer() { return mpBuffer; }

	static VuD3d11IndexBuffer	*create(int count, VUUINT32 usageFlags);

	ID3D11Buffer			*mpD3dIndexBuffer;
	VUUINT16				*mpBuffer; // maintain shadow copy to handle lost device
	D3D11_BUFFER_DESC		mDesc;
};
