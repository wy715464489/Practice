//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Functionality to deal with binary data.
// 
//*****************************************************************************

#include "VuBinaryDataUtil.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuQuaternion.h"


//*****************************************************************************
void VuBinaryDataWriter::configure(const std::string &platform)
{
	if ( platform == "Win32" )
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
	}
	else if ( platform == "Android" )
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
	}
	else if ( platform == "Ios" )
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
	}
	else if (platform == "Windows")
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
	}
	else if (platform == "Ps4")
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
	}
	else if (platform == "Xb1")
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
	}
	else
	{
		mSwapEndian = !VU_LITTLE_ENDIAN;
		VUASSERT(0, "Unknown platform!");
	}
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuAabb &value)
{
	writeValue(value.mMin);
	writeValue(value.mMax);
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuRect &value)
{
	writeValue(value.mX);
	writeValue(value.mY);
	writeValue(value.mWidth);
	writeValue(value.mHeight);
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuVector2 &value)
{
	writeValue(value.mX);
	writeValue(value.mY);
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuVector3 &value)
{
	writeValue(value.mX);
	writeValue(value.mY);
	writeValue(value.mZ);
	writeValue(value.mPad);
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuVector4 &value)
{
	writeValue(value.mX);
	writeValue(value.mY);
	writeValue(value.mZ);
	writeValue(value.mW);
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuMatrix &value)
{
	writeValue(value.mX);
	writeValue(value.mY);
	writeValue(value.mZ);
	writeValue(value.mT);
}

//*****************************************************************************
void VuBinaryDataWriter::writeValue(const VuQuaternion &value)
{
	writeValue(value.mVec);
}

//*****************************************************************************
void VuBinaryDataWriter::writeArray(const VuArray<VuVector3> &value)
{
	int count = value.size();
	writeValue(count);
	writeData(&value[0], count*sizeof(value[0]));
	if ( mSwapEndian )
		for ( int i = count*4; i > 0; i-- )
			VuEndianUtil::swapInPlace(*(((float *)&mData.end()) - i));
}

//*****************************************************************************
void VuBinaryDataReader::attach(const void *pData, int size)
{
	mpData = (VUBYTE *)pData;
	mSize = size;
	mOffset = 0;
}

//*****************************************************************************
void VuBinaryDataReader::attach(const VuArray<VUBYTE> &data)
{
	mpData = &data.begin();
	mSize = data.size();
	mOffset = 0;
}
