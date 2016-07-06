//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Binary data inline functionality.
// 
//*****************************************************************************


#include "VuEngine/Util/VuEndianUtil.h"


//*****************************************************************************
void VuBinaryDataWriter::writeValue(const float &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((float *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const bool &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((bool *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUINT8 &value)
{
	writeData(&value, sizeof(value));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUINT16 &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((VUINT16 *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUINT32 &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((VUINT32 *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUINT64 &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((VUINT64 *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUUINT8 &value)
{
	writeData(&value, sizeof(value));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUUINT16 &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((VUUINT16 *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUUINT32 &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((VUUINT32 *)&mData.end()) - 1));
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VUUINT64 &value)
{
	writeData(&value, sizeof(value));
	if ( mSwapEndian )
		VuEndianUtil::swapInPlace(*(((VUUINT64 *)&mData.end()) - 1));
}

//*****************************************************************************
template<class T>
inline void VuBinaryDataWriter::writeValueCompat(const T &value)
{
#if VU_LITTLE_ENDIAN
	T tempValue;
	VuEndianUtil::swap(value, tempValue);
	writeData(&tempValue, sizeof(value));
#else
	writeData(&value, sizeof(value));
#endif
}

//*****************************************************************************
void VuBinaryDataWriter::writeData(const void *pData, int size)
{
	VU_MEMCPY(allocate(size), size, pData, size);
}

//*****************************************************************************
void *VuBinaryDataWriter::allocate(int size)
{
	int offset = mData.size();
	mData.resize(offset + size);
	return &mData[offset];
}

//*****************************************************************************
void VuBinaryDataWriter::reserve(int size)
{
	int offset = mData.size();
	mData.reserve(offset + size);
}

//*****************************************************************************
void VuBinaryDataWriter::writeString(const std::string &value)
{
	writeData(value.c_str(), (int)value.length() + 1);
}

//*****************************************************************************
void VuBinaryDataWriter::writeString(const char *str)
{
	writeData(str, (int)strlen(str) + 1);
}

//*****************************************************************************
void VuBinaryDataWriter::writeArray(const VuArray<int> &value)
{
	int count = value.size();
	writeValue(count);
	if ( count )
	{
		writeData(&value[0], count*sizeof(value[0]));
		if ( mSwapEndian )
			for ( int i = count; i > 0; i-- )
				VuEndianUtil::swapInPlace(*(((int *)&mData.end()) - i));
	}
}

//*****************************************************************************
void VuBinaryDataWriter::writeArray(const VuArray<float> &value)
{
	int count = value.size();
	writeValue(count);
	if ( count )
	{
		writeData(&value[0], count*sizeof(value[0]));
		if ( mSwapEndian )
			for ( int i = count; i > 0; i-- )
				VuEndianUtil::swapInPlace(*(((float *)&mData.end()) - i));
	}
}

//*****************************************************************************
void VuBinaryDataWriter::writeArray(const VuArray<VUUINT8> &value)
{
	int count = value.size();
	writeValue(count);
	if ( count )
	{
		writeData(&value[0], count*sizeof(value[0]));
	}
}

//*****************************************************************************
void VuBinaryDataWriter::writeArray(const VuArray<VUUINT16> &value)
{
	int count = value.size();
	writeValue(count);
	if ( count )
	{
		writeData(&value[0], count*sizeof(value[0]));
		if ( mSwapEndian )
			for ( int i = count; i > 0; i-- )
				VuEndianUtil::swapInPlace(*(((VUUINT16 *)&mData.end()) - i));
	}
}

//*****************************************************************************
void VuBinaryDataWriter::writeArray(const VuArray<VUUINT32> &value)
{
	int count = value.size();
	writeValue(count);
	if ( count )
	{
		writeData(&value[0], count*sizeof(value[0]));
		if ( mSwapEndian )
			for ( int i = count; i > 0; i-- )
				VuEndianUtil::swapInPlace(*(((VUUINT32 *)&mData.end()) - i));
	}
}

//*****************************************************************************
template<class T>
void VuBinaryDataWriter::writeArrayCompat(const VuArray<T> &value)
{
#if VU_LITTLE_ENDIAN
	int count = value.size();
	writeValueCompat(count);
	for ( int i = 0; i < count; i++ )
		writeValueCompat(value[i]);
#else
	int count = value.size();
	writeData(&count, sizeof(count));
	if ( count )
	{
		writeData(&value[0], count*sizeof(value[0]));
	}
#endif
}

//*****************************************************************************
template<class T>
void VuBinaryDataReader::readValueCompat(T &value)
{
	readValue(value);
#if VU_LITTLE_ENDIAN
	VuEndianUtil::swapInPlace(value);
#endif
}

//*****************************************************************************
void VuBinaryDataReader::readData(void *pData, int size)
{
	VUASSERT(mOffset + size <= mSize, "VuBinaryReader::read() overflow");
	VU_MEMCPY(pData, size, &mpData[mOffset], size);
	mOffset += size;
}

//*****************************************************************************
void VuBinaryDataReader::skip(int size)
{
	VUASSERT(mOffset + size <= mSize, "VuBinaryReader::read() overflow");
	mOffset += size;
}

//*****************************************************************************
void VuBinaryDataReader::readString(std::string &value)
{
	value = (const char *)&mpData[mOffset];
	mOffset += (int)value.length() + 1;
}

//*****************************************************************************
const char *VuBinaryDataReader::readString()
{
	const char *str = (const char *)&mpData[mOffset];
	mOffset += (int)strlen(str) + 1;
	return str;
}

//*****************************************************************************
template<class T>
void VuBinaryDataReader::readArray(VuArray<T> &value)
{
	int count;
	readValue(count);
	value.resize(count);
	if ( count )
	{
		readData(&value[0], count*sizeof(value[0]));
	}
}

//*****************************************************************************
template<class T>
void VuBinaryDataReader::readArrayCompat(VuArray<T> &value)
{
#if VU_LITTLE_ENDIAN
	int count;
	readValueCompat(count);
	value.resize(count);
	for ( int i = 0; i < count; i++ )
		readValueCompat(value[i]);
#else
	readArray(value);
#endif
}
