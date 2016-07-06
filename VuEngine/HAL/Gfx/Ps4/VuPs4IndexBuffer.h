//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the index buffer interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"


class VuPs4IndexBuffer : public VuIndexBuffer
{
public:
	VuPs4IndexBuffer(int count);
	~VuPs4IndexBuffer();

	virtual void			resize(int newCount);
	virtual void			setData(const VUUINT16 *pData, int count);
	virtual const VUUINT16	*getShadowBuffer() { return VUNULL; }

	static VuPs4IndexBuffer	*create(int count, VUUINT32 usageFlags);

	void		*mpBuffer;
};
