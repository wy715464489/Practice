//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to read data containers in binary format.
// 
//*****************************************************************************

#include <stdarg.h>
#include "VuJsonBinaryReader.h"
#include "VuJsonContainer.h"
#include "VuEngine/HAL/File/VuFile.h"


static const unsigned int scMagic = ('V'<<24)|('U'<<16)|('J'<<8)|('B');
static const unsigned int scVersion = 1;

#define MAX_ARRAY_SIZE 65536


//*****************************************************************************
VuJsonBinaryReader::VuJsonBinaryReader():
	mpDataPtr(0),
	mDataRemaining(0)
{
}

//*****************************************************************************
bool VuJsonBinaryReader::loadFromFile(VuJsonContainer &container, const std::string &strFileName)
{
	mstrError.clear();

	// open file
	VUHANDLE fp = VuFile::IF()->open(strFileName, VuFile::MODE_READ);
	if ( fp == VUNULL )
		return error("Unable to open for reading: %s", strFileName.c_str());

	// get file size
	int dataSize =  VuFile::IF()->size(fp);

	// allocate memory
	char *pData = new char[dataSize];

	// read file
	VuFile::IF()->read(fp, pData, dataSize);

	// load from memory
	bool bSuccess = loadFromMemory(container, pData, dataSize);

	// clean up
	delete[] pData;
	VuFile::IF()->close(fp);

	return bSuccess;
}

//*****************************************************************************
bool VuJsonBinaryReader::loadFromMemory(VuJsonContainer &container, const void *pData, int dataSize)
{
	mstrError.clear();
	mpDataPtr = (const unsigned char *)pData;
	mDataRemaining = dataSize;

	// read header
	unsigned int magic = 0, version = 0;
	if ( !readValue(magic) )	return false;
	if ( !readValue(version) )	return false;
	if ( magic != scMagic )		return error("Magic mismatch");
	if ( version != scVersion )	return error("Version mismatch");

	// start w/ empty container
	container.clear();

	// recursively read containers
	if ( !readContainer(container) )
	{
		container.clear();
		return false;
	}

	return true;
}

//*****************************************************************************
const std::string &VuJsonBinaryReader::getLastError()
{
	return mstrError;
}

//*****************************************************************************
bool VuJsonBinaryReader::error(const char *fmt, ...)
{
	va_list args;
	char str[256];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	mstrError += str;
	mstrError += '\n';

	return false;
}

//*****************************************************************************
bool VuJsonBinaryReader::readContainer(VuJsonContainer &container)
{
	// get type
	if ( !readValue(container.mType) )
		return false;
	
	switch ( container.mType )
	{
		case VuJsonContainer::nullValue:
		{
			break;
		}
		case VuJsonContainer::intValue:
		{
			if ( !readValue(container.mValue.mInt) )
				return false;
			break;
		}
		case VuJsonContainer::floatValue:
		{
			if ( !readValue(container.mValue.mFloat) )
				return false;
			break;
		}
		case VuJsonContainer::boolValue:
		{
			if ( !readValue(container.mValue.mBool) )
				return false;
			break;
		}
		case VuJsonContainer::stringValue:
		{
			if ( !readString() )
				return false;
			container.mValue.mpString = new VuJsonContainer::String;
			container.mValue.mpString->assign(&mStringBuffer[0], mStringBuffer.size() - 1);
			break;
		}
		case VuJsonContainer::arrayValue:
		{
			int size = 0;
			if ( !readValue(size) )
				return false;
			if ( size > MAX_ARRAY_SIZE )
				return error("Max array size exceeded.");
			container.mValue.mpArray = new VuJsonContainer::Array;
			container.mValue.mpArray->resize(size);
			for ( int index = 0; index < size; index++ )
				if ( !readContainer((*container.mValue.mpArray)[index]) )
					return false;
			break;
		}
		case VuJsonContainer::objectValue:
		{
			int count = 0;
			if ( !readValue(count) )
				return false;
			container.mValue.mpObject = new VuJsonContainer::Object;
			for ( int member = 0; member < count; member++ )
			{
				if ( !readString() )
					return false;
				VUUINT64 key = VuHash::fnv64String(&mStringBuffer[0]);
				VuJsonContainer::MapValue &value = (*container.mValue.mpObject)[key];
				value.mKey = &mStringBuffer[0];
				if ( !readContainer(value.mValue) )
					return false;
			}
			break;
		}
		case VuJsonContainer::int64Value:
		{
			if ( !readValue(container.mValue.mInt64) )
				return false;
			break;
		}
		case VuJsonContainer::binaryValue:
		{
			int size;
			if ( !readValue(size) )
				return false;

			container.mValue.mBinary.mSize = size;
			container.mValue.mBinary.mpData = malloc(size);

			if ( size > mDataRemaining )
				return false;

			VU_MEMCPY(container.mValue.mBinary.mpData, size, mpDataPtr, size);
			mpDataPtr += size;
			mDataRemaining -= size;
			break;
		}
		default:
		{
			return error("Unknown container type.");
			break;
		}
	}

	return true;
}

//*****************************************************************************
bool VuJsonBinaryReader::readString()
{
	int len;
	if ( !readValue(len) )
		return false;

	if ( len > mDataRemaining )
		return("Read error");

	mStringBuffer.resize(len + 1);

	VU_MEMCPY(&mStringBuffer[0], mStringBuffer.size(), mpDataPtr, len);
	mpDataPtr += len;
	mDataRemaining -= len;

	mStringBuffer[len] = '\0';

	return true;
}
