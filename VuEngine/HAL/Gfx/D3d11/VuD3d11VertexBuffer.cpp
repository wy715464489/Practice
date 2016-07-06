//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the vertex buffer interface class.
//
//*****************************************************************************

#include "VuD3d11VertexBuffer.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"

// static variables
typedef std::list<VuD3d11VertexBuffer *> VertexBuffers;
static VertexBuffers sVertexBuffers;


//*****************************************************************************
VuD3d11VertexBuffer::VuD3d11VertexBuffer(int size):
	VuVertexBuffer(size),
	mpD3dVertexBuffer(0),
	mpBuffer(0)
{
	sVertexBuffers.push_back(this);
}

//*****************************************************************************
VuD3d11VertexBuffer::~VuD3d11VertexBuffer()
{
	sVertexBuffers.remove(this);

	mpD3dVertexBuffer->Release();
	delete[] mpBuffer;
}

//*****************************************************************************
void VuD3d11VertexBuffer::resize(int newSize)
{
	// delete old buffer
	mpD3dVertexBuffer->Release();

	// create new buffer
	mDesc.ByteWidth = newSize;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateBuffer(&mDesc, NULL, &mpD3dVertexBuffer));

	// update size
	mSize = newSize;

	// update shadow buffer size
	if ( mpBuffer )
	{
		delete[] mpBuffer;
		mpBuffer = new VUBYTE[newSize];
	}
}

//*****************************************************************************
void VuD3d11VertexBuffer::setData(const void *pData, int size)
{
	if ( mpBuffer )
	{
		VU_MEMCPY(mpBuffer, mSize, pData, size);

		VuD3d11Gfx::IF()->getD3dDeviceContext()->UpdateSubresource(mpD3dVertexBuffer, 0, NULL, pData, size, 0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		D3DCALL(VuD3d11Gfx::IF()->getD3dDeviceContext()->Map(mpD3dVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		VU_MEMCPY(mappedResource.pData, mSize, pData, size);
		VuD3d11Gfx::IF()->getD3dDeviceContext()->Unmap(mpD3dVertexBuffer, 0);
	}
}

//*****************************************************************************
VuD3d11VertexBuffer *VuD3d11VertexBuffer::create(int size, VUUINT32 usageFlags)
{
	// allocate buffer
	VuD3d11VertexBuffer *pVertexBuffer = new VuD3d11VertexBuffer(size);

	pVertexBuffer->mDesc.ByteWidth = size;
	pVertexBuffer->mDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	pVertexBuffer->mDesc.MiscFlags = 0;
	pVertexBuffer->mDesc.StructureByteStride = 0;

	// translate usage
	if ( usageFlags & VUGFX_USAGE_DYNAMIC )
	{
		pVertexBuffer->mDesc.Usage = D3D11_USAGE_DYNAMIC;
		pVertexBuffer->mDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		pVertexBuffer->mDesc.Usage = D3D11_USAGE_DEFAULT;
		pVertexBuffer->mDesc.CPUAccessFlags = 0;
		pVertexBuffer->mpBuffer = new VUBYTE[size];
	}

	// create buffer
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateBuffer(&pVertexBuffer->mDesc, NULL, &pVertexBuffer->mpD3dVertexBuffer));

	return pVertexBuffer;
}
