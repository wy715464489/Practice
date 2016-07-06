//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  btOptimizedBvh wrapper
// 
//*****************************************************************************

#include "VuOptimizedBvh.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
void VuOptimizedBvh::serialize(VuBinaryDataWriter &writer)
{
	writer.writeValue(m_bvhAabbMin[0]);
	writer.writeValue(m_bvhAabbMin[1]);
	writer.writeValue(m_bvhAabbMin[2]);
	writer.writeValue(m_bvhAabbMin[3]);

	writer.writeValue(m_bvhAabbMax[0]);
	writer.writeValue(m_bvhAabbMax[1]);
	writer.writeValue(m_bvhAabbMax[2]);
	writer.writeValue(m_bvhAabbMax[3]);

	writer.writeValue(m_bvhQuantization[0]);
	writer.writeValue(m_bvhQuantization[1]);
	writer.writeValue(m_bvhQuantization[2]);
	writer.writeValue(m_bvhQuantization[3]);

	writer.writeValue(m_bulletVersion);
	writer.writeValue(m_curNodeIndex);
	writer.writeValue(m_useQuantization);

	writeArray(writer, m_leafNodes);
	writeArray(writer, m_contiguousNodes);
	writeArray(writer, m_quantizedLeafNodes);
	writeArray(writer, m_quantizedContiguousNodes);

	writer.writeValue(m_traversalMode);
	writeArray(writer, m_SubtreeHeaders);

	writer.writeValue(m_subtreeHeaderCount);
}

//*****************************************************************************
void VuOptimizedBvh::deserialize(VuBinaryDataReader &reader)
{
	reader.readValue(m_bvhAabbMin[0]);
	reader.readValue(m_bvhAabbMin[1]);
	reader.readValue(m_bvhAabbMin[2]);
	reader.readValue(m_bvhAabbMin[3]);

	reader.readValue(m_bvhAabbMax[0]);
	reader.readValue(m_bvhAabbMax[1]);
	reader.readValue(m_bvhAabbMax[2]);
	reader.readValue(m_bvhAabbMax[3]);

	reader.readValue(m_bvhQuantization[0]);
	reader.readValue(m_bvhQuantization[1]);
	reader.readValue(m_bvhQuantization[2]);
	reader.readValue(m_bvhQuantization[3]);

	reader.readValue(m_bulletVersion);
	reader.readValue(m_curNodeIndex);
	reader.readValue(m_useQuantization);

	readArray(reader, m_leafNodes);
	readArray(reader, m_contiguousNodes);
	readArray(reader, m_quantizedLeafNodes);
	readArray(reader, m_quantizedContiguousNodes);

	reader.readValue(m_traversalMode);
	readArray(reader, m_SubtreeHeaders);

	reader.readValue(m_subtreeHeaderCount);
}

//*****************************************************************************
void VuOptimizedBvh::writeArray(VuBinaryDataWriter &writer, const btAlignedObjectArray<btOptimizedBvhNode> &array)
{
	int capacity = array.capacity();
	int size = array.size();
	writer.writeValue(capacity);
	writer.writeValue(size);
	for ( int i = 0; i < size; i++ )
	{
		writer.writeValue(array[i].m_aabbMinOrg[0]);
		writer.writeValue(array[i].m_aabbMinOrg[1]);
		writer.writeValue(array[i].m_aabbMinOrg[2]);
		writer.writeValue(array[i].m_aabbMinOrg[3]);

		writer.writeValue(array[i].m_aabbMaxOrg[0]);
		writer.writeValue(array[i].m_aabbMaxOrg[1]);
		writer.writeValue(array[i].m_aabbMaxOrg[2]);
		writer.writeValue(array[i].m_aabbMaxOrg[3]);

		writer.writeValue(array[i].m_escapeIndex);

		writer.writeValue(array[i].m_subPart);
		writer.writeValue(array[i].m_triangleIndex);
		writer.writeValue(array[i].m_padding[0]);
		writer.writeValue(array[i].m_padding[1]);
		writer.writeValue(array[i].m_padding[2]);
		writer.writeValue(array[i].m_padding[3]);
		writer.writeValue(array[i].m_padding[4]);
	}
}

//*****************************************************************************
void VuOptimizedBvh::writeArray(VuBinaryDataWriter &writer, const btAlignedObjectArray<btQuantizedBvhNode> &array)
{
	int capacity = array.capacity();
	int size = array.size();
	writer.writeValue(capacity);
	writer.writeValue(size);
	for ( int i = 0; i < size; i++ )
	{
		writer.writeValue(array[i].m_quantizedAabbMin[0]);
		writer.writeValue(array[i].m_quantizedAabbMin[1]);
		writer.writeValue(array[i].m_quantizedAabbMin[2]);

		writer.writeValue(array[i].m_quantizedAabbMax[0]);
		writer.writeValue(array[i].m_quantizedAabbMax[1]);
		writer.writeValue(array[i].m_quantizedAabbMax[2]);

		writer.writeValue(array[i].m_escapeIndexOrTriangleIndex);
	}
}

//*****************************************************************************
void VuOptimizedBvh::writeArray(VuBinaryDataWriter &writer, const btAlignedObjectArray<btBvhSubtreeInfo> &array)
{
	int capacity = array.capacity();
	int size = array.size();
	writer.writeValue(capacity);
	writer.writeValue(size);
	for ( int i = 0; i < size; i++ )
	{
		writer.writeValue(array[i].m_quantizedAabbMin[0]);
		writer.writeValue(array[i].m_quantizedAabbMin[1]);
		writer.writeValue(array[i].m_quantizedAabbMin[2]);

		writer.writeValue(array[i].m_quantizedAabbMax[0]);
		writer.writeValue(array[i].m_quantizedAabbMax[1]);
		writer.writeValue(array[i].m_quantizedAabbMax[2]);

		writer.writeValue(array[i].m_rootNodeIndex);

		writer.writeValue(array[i].m_subtreeSize);
		writer.writeValue(array[i].m_padding[0]);
		writer.writeValue(array[i].m_padding[1]);
		writer.writeValue(array[i].m_padding[2]);
	}
}

//*****************************************************************************
template <typename T>
void VuOptimizedBvh::readArray(VuBinaryDataReader &reader, btAlignedObjectArray<T> &array, const T &fillData)
{
	int size, capacity;
	reader.readValue(capacity);
	reader.readValue(size);
	array.reserve(capacity);
	array.resize(size, fillData);
	if ( size )
		reader.readData(&array[0], size*sizeof(array[0]));
}