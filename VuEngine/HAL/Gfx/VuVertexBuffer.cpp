//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VertexBuffer interface class.
// 
//*****************************************************************************

#include "VuVertexBuffer.h"
#include "VuVertexDeclaration.h"
#include "VuGfx.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuEndianUtil.h"


//*****************************************************************************
bool VuVertexBuffer::bake(const std::vector<VUBYTE> &data, VuBinaryDataWriter &writer)
{
	int size = (int)data.size();
	writer.writeValue(size);
	writer.writeData(&data[0], size);

	return true;
}

//*****************************************************************************
VuVertexBuffer *VuVertexBuffer::load(VuBinaryDataReader &reader)
{
	int size;
	reader.readValue(size);

	// create verex buffer
	VuVertexBuffer *pVertexBuffer = VuGfx::IF()->createVertexBuffer(size, 0);

	// read data
	pVertexBuffer->setData(reader.cur(), size);
	reader.skip(size);

	return pVertexBuffer;
}

//*****************************************************************************
void VuVertexBuffer::endianSwap(VUBYTE *pData, int vertexCount, int vertexStride, const VuVertexDeclarationElements &elements)
{
	const VuVertexDeclarationElement *pElements = &elements.front();

	int vertexSize = elements.calcVertexSize(0);
	int elementCount = (int)elements.size();

	int byteCount = 0;
	for ( int iVert = 0; iVert < vertexCount; iVert++ )
	{
		for ( int iElement = 0; iElement < elementCount; iElement++ )
		{
			switch( pElements[iElement].mType )
			{
				case VUGFX_DECL_TYPE_FLOAT1:
				{
					float *p = reinterpret_cast<float *>(pData + byteCount);
					VuEndianUtil::swapInPlace(p[0]);
					byteCount += 4;
					break;
				}
				case VUGFX_DECL_TYPE_FLOAT2:
				{
					float *p = reinterpret_cast<float *>(pData + byteCount);
					VuEndianUtil::swapInPlace(p[0]);
					VuEndianUtil::swapInPlace(p[1]);
					byteCount += 8;
					break;
				}
				case VUGFX_DECL_TYPE_FLOAT3:
				{
					float *p = reinterpret_cast<float *>(pData + byteCount);
					VuEndianUtil::swapInPlace(p[0]);
					VuEndianUtil::swapInPlace(p[1]);
					VuEndianUtil::swapInPlace(p[2]);
					byteCount += 12;
					break;
				}
				case VUGFX_DECL_TYPE_FLOAT4:
				{
					float *p = reinterpret_cast<float *>(pData + byteCount);
					VuEndianUtil::swapInPlace(p[0]);
					VuEndianUtil::swapInPlace(p[1]);
					VuEndianUtil::swapInPlace(p[2]);
					VuEndianUtil::swapInPlace(p[3]);
					byteCount += 16;
					break;
				}
				case VUGFX_DECL_TYPE_UBYTE4:
				case VUGFX_DECL_TYPE_UBYTE4N:
				case VUGFX_DECL_TYPE_SHORT2N:
				case VUGFX_DECL_TYPE_FLOAT16_2:
				{
					VUUINT16 *p = reinterpret_cast<VUUINT16 *>(pData + byteCount);
					VuEndianUtil::swapInPlace(p[0]);
					VuEndianUtil::swapInPlace(p[1]);
					byteCount += 4;
					break;
				}
				case VUGFX_DECL_TYPE_SHORT4N:
				case VUGFX_DECL_TYPE_FLOAT16_4:
				{
					VUUINT16 *p = reinterpret_cast<VUUINT16 *>(pData + byteCount);
					VuEndianUtil::swapInPlace(p[0]);
					VuEndianUtil::swapInPlace(p[1]);
					VuEndianUtil::swapInPlace(p[2]);
					VuEndianUtil::swapInPlace(p[3]);
					byteCount += 8;
					break;
				}
				default:
					VUASSERT(0, "VuVertexBuffer::endianSwap() unsupported type");
			}
		}

		pData += vertexStride - vertexSize;
	}
}