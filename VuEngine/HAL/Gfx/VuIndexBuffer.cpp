//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  IndexBuffer interface class.
// 
//*****************************************************************************

#include "VuIndexBuffer.h"
#include "VuGfx.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuEndianUtil.h"


//*****************************************************************************
bool VuIndexBuffer::bake(const std::vector<VUUINT16> &data, VuBinaryDataWriter &writer)
{
	int count = (int)data.size();
	writer.writeValue(count);
	writer.writeData(&data[0], count*sizeof(data[0]));

	return true;
}

//*****************************************************************************
VuIndexBuffer *VuIndexBuffer::load(VuBinaryDataReader &reader)
{
	int count;
	reader.readValue(count);

	// create index buffer
	VuIndexBuffer *pIndexBuffer = VuGfx::IF()->createIndexBuffer(count, 0);

	// read data
	pIndexBuffer->setData(static_cast<const VUUINT16 *>(reader.cur()), count);
	reader.skip(count*2);

	return pIndexBuffer;
}