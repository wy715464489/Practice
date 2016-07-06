//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the index buffer interface class.
//
//*****************************************************************************

#include "VuD3d11IndexBuffer.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuD3d11IndexBuffer::VuD3d11IndexBuffer(int count):
	VuIndexBuffer(count),
	mpD3dIndexBuffer(0),
	mpBuffer(0)
{
}

//*****************************************************************************
VuD3d11IndexBuffer::~VuD3d11IndexBuffer()
{
	mpD3dIndexBuffer->Release();
	delete[] mpBuffer;
}

//*****************************************************************************
void VuD3d11IndexBuffer::resize(int newCount)
{
	// delete old buffer
	mpD3dIndexBuffer->Release();

	// create new buffer
	mDesc.ByteWidth = newCount*2;
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateBuffer(&mDesc, NULL, &mpD3dIndexBuffer));

	// update size
	mCount = newCount;

	// update shadow buffer size
	if ( mpBuffer )
	{
		delete[] mpBuffer;
		mpBuffer = new VUUINT16[newCount];
	}
}

//*****************************************************************************
void VuD3d11IndexBuffer::setData(const VUUINT16 *pData, int count)
{
	if ( mpBuffer )
	{
		VU_MEMCPY(mpBuffer, mCount*2, pData, count*2);

		VuD3d11Gfx::IF()->getD3dDeviceContext()->UpdateSubresource(mpD3dIndexBuffer, 0, NULL, pData, count*2, 0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		D3DCALL(VuD3d11Gfx::IF()->getD3dDeviceContext()->Map(mpD3dIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		VU_MEMCPY(mappedResource.pData, mCount*2, pData, count*2);
		VuD3d11Gfx::IF()->getD3dDeviceContext()->Unmap(mpD3dIndexBuffer, 0);
	}
}

//*****************************************************************************
VuD3d11IndexBuffer *VuD3d11IndexBuffer::create(int count, VUUINT32 usageFlags)
{
	// allocate buffer
	VuD3d11IndexBuffer *pIndexBuffer = new VuD3d11IndexBuffer(count);

	pIndexBuffer->mDesc.ByteWidth = count*2;
	pIndexBuffer->mDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	pIndexBuffer->mDesc.MiscFlags = 0;
	pIndexBuffer->mDesc.StructureByteStride = 0;

	// translate usage
	if ( usageFlags & VUGFX_USAGE_DYNAMIC )
	{
		pIndexBuffer->mDesc.Usage = D3D11_USAGE_DYNAMIC;
		pIndexBuffer->mDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		pIndexBuffer->mDesc.Usage = D3D11_USAGE_DEFAULT;
		pIndexBuffer->mDesc.CPUAccessFlags = 0;
		pIndexBuffer->mpBuffer = new VUUINT16[count];
	}

	// create buffer
	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateBuffer(&pIndexBuffer->mDesc, NULL, &pIndexBuffer->mpD3dIndexBuffer));

	return pIndexBuffer;
}