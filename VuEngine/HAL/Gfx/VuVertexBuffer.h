//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VertexBuffer interface class.
// 
//*****************************************************************************

#pragma once

#include "VuGfxTypes.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuAabb.h"

class VuJsonContainer;
class VuVertexDeclarationElements;
class VuBinaryDataReader;
class VuBinaryDataWriter;

class VuVertexBuffer : public VuRefObj
{
public:
	VuVertexBuffer(int size) : mSize(size) {}

	virtual void			resize(int newSize) = 0;
	virtual void			setData(const void *pData, int size) = 0;
	virtual const void		*getShadowBuffer() = 0;

	static bool				bake(const std::vector<VUBYTE> &data, VuBinaryDataWriter &writer);
	static VuVertexBuffer	*load(VuBinaryDataReader &reader);
	static void				endianSwap(VUBYTE *pData, int vertexCount, int vertexStride, const VuVertexDeclarationElements &elements);

	int						mSize;
};
