//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  btOptimizedBvh wrapper
// 
//*****************************************************************************

#pragma once

#include "btBulletDynamicsCommon.h"

class VuBinaryDataWriter;
class VuBinaryDataReader;

class VuOptimizedBvh : public btOptimizedBvh
{
public:
	void	serialize(VuBinaryDataWriter &writer);
	void	deserialize(VuBinaryDataReader &reader);

	virtual bool serialize(void *o_alignedDataBuffer, unsigned i_dataBufferSize, bool i_swapEndian) const { return btQuantizedBvh::serialize(o_alignedDataBuffer, i_dataBufferSize, i_swapEndian); }
	virtual	const char*	serialize(void* dataBuffer, btSerializer* serializer) const { return btQuantizedBvh::serialize(dataBuffer, serializer); }

private:
	static void	writeArray(VuBinaryDataWriter &writer, const btAlignedObjectArray<btOptimizedBvhNode> &array);
	static void	writeArray(VuBinaryDataWriter &writer, const btAlignedObjectArray<btQuantizedBvhNode> &array);
	static void	writeArray(VuBinaryDataWriter &writer, const btAlignedObjectArray<btBvhSubtreeInfo> &array);

	template <typename T>
	static void	readArray(VuBinaryDataReader &reader, btAlignedObjectArray<T> &array, const T &fillData = T());
};
