//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the vertex buffer interface class.
// 
//*****************************************************************************

#pragma once

#ifdef VUXB1
	#include <d3d11_x.h>
#else
	#include <d3d11.h>
#endif

#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"


class VuD3d11VertexBuffer : public VuVertexBuffer
{
public:
	VuD3d11VertexBuffer(int size);
	~VuD3d11VertexBuffer();

	virtual void			resize(int newSize);
	virtual void			setData(const void *pData, int size);
	virtual const void		*getShadowBuffer() { return mpBuffer; }

	static VuD3d11VertexBuffer	*create(int size, VUUINT32 usageFlags);

	ID3D11Buffer			*mpD3dVertexBuffer;
	VUBYTE					*mpBuffer; // maintain shadow copy to handle lost device
	D3D11_BUFFER_DESC		mDesc;
};
