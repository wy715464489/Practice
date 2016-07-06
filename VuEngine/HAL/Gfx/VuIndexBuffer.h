//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  IndexBuffer interface class.
// 
//*****************************************************************************

#pragma once

#include "VuGfxTypes.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"

class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;


class VuIndexBuffer : public VuRefObj
{
public:
	VuIndexBuffer(int count) : mCount(count) {}

	int						getIndexCount() { return mCount; }

	virtual void			resize(int newCount) = 0;
	virtual void			setData(const VUUINT16 *pData, int count) = 0;
	virtual const VUUINT16	*getShadowBuffer() = 0;

	static bool				bake(const std::vector<VUUINT16> &data, VuBinaryDataWriter &writer);
	static VuIndexBuffer	*load(VuBinaryDataReader &reader);

	int						mCount;
};
