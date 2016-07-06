//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to write data containers in binary format.
// 
//*****************************************************************************

#include "VuJsonBinaryWriter.h"
#include "VuJsonContainer.h"
#include "VuEngine/HAL/File/VuFile.h"


static const unsigned int scMagic = ('V'<<24)|('U'<<16)|('J'<<8)|('B');
static const unsigned int scVersion = 1;


//*****************************************************************************
VuJsonBinaryWriter::VuJsonBinaryWriter():
	mpDataPtr(0),
	mDataRemaining(0)
{
}

//*****************************************************************************
bool VuJsonBinaryWriter::saveToFile(const VuJsonContainer &container, const std::string &strFileName)
{
	// open file
	VUHANDLE fp = VuFile::IF()->open(strFileName, VuFile::MODE_WRITE);
	if ( fp == VUNULL )
		return false;

	// calculate size
	int dataSize = calculateDataSize(container);

	// allocate memory
	char *pData = new char[dataSize];

	// write to memory
	bool bSuccess = saveToMemory(container, pData, dataSize);

	// write file
	VuFile::IF()->write(fp, pData, dataSize);

	// clean up
	delete[] pData;
	VuFile::IF()->close(fp);

	return bSuccess;
}

//*****************************************************************************
bool VuJsonBinaryWriter::saveToMemory(const VuJsonContainer &container, void *pData, int &dataSize)
{
	mpDataPtr = (unsigned char *)pData;
	mDataRemaining = dataSize;

	// write header
	if ( !writeValue(scMagic) )		return false;
	if ( !writeValue(scVersion) )	return false;

	// recursively write containers
	if ( !writeContainer(container) )
		return false;

	// adjust dataSize to what was actually written
	dataSize -= mDataRemaining;

	return true;
}

//*****************************************************************************
int VuJsonBinaryWriter::calculateDataSize(const VuJsonContainer &container)
{
	int dataSize = 8; // header

	dataSize += calculateContainerDataSize(container);

	return dataSize;
}

//*****************************************************************************
int VuJsonBinaryWriter::calculateContainerDataSize(const VuJsonContainer &container)
{
	int dataSize = sizeof(container.mType);

	switch ( container.mType )
	{
		case VuJsonContainer::nullValue:
		{
			break;
		}
		case VuJsonContainer::intValue:
		{
			dataSize += sizeof(container.mValue.mInt);
			break;
		}
		case VuJsonContainer::floatValue:
		{
			dataSize += sizeof(container.mValue.mFloat);
			break;
		}
		case VuJsonContainer::boolValue:
		{
			dataSize += sizeof(container.mValue.mBool);
			break;
		}
		case VuJsonContainer::stringValue:
		{
			int len = (int)container.mValue.mpString->length();
			dataSize += sizeof(len);
			dataSize += len;
			break;
		}
		case VuJsonContainer::arrayValue:
		{
			int size = (int)container.mValue.mpArray->size();
			dataSize += sizeof(size);
			for ( int index = 0; index < size; index++ )
				dataSize += calculateContainerDataSize((*container.mValue.mpArray)[index]);
			break;
		}
		case VuJsonContainer::objectValue:
		{
			//int count = container.numMembers();
			//dataSize += sizeof(count);
			dataSize += sizeof(int);
			for ( VuJsonContainer::Object::const_iterator iter = container.mValue.mpObject->begin(); iter != container.mValue.mpObject->end(); iter++ )
			{
				int nameSize = (int)iter->second.mKey.length();
				dataSize += sizeof(nameSize);
				dataSize += nameSize;
				dataSize += calculateContainerDataSize(iter->second.mValue);
			}
			break;
		}
		case VuJsonContainer::int64Value:
		{
			dataSize += sizeof(container.mValue.mInt64);
			break;
		}
		case VuJsonContainer::binaryValue:
		{
			int size = (int)container.mValue.mBinary.mSize;
			dataSize += sizeof(size);
			dataSize += size;
			break;
		}
		default:
		{
			VUASSERT(0, "Unknown Json container type");
			break;
		}
	}

	return dataSize;
}

//*****************************************************************************
bool VuJsonBinaryWriter::writeContainer(const VuJsonContainer &container)
{
	// write type
	if ( !writeValue(container.mType) )
		return false;
	
	switch ( container.mType )
	{
		case VuJsonContainer::nullValue:
		{
			break;
		}
		case VuJsonContainer::intValue:
		{
			if ( !writeValue(container.mValue.mInt) )
				return false;
			break;
		}
		case VuJsonContainer::floatValue:
		{
			if ( !writeValue(container.mValue.mFloat) )
				return false;
			break;
		}
		case VuJsonContainer::boolValue:
		{
			if ( !writeValue(container.mValue.mBool) )
				return false;
			break;
		}
		case VuJsonContainer::stringValue:
		{
			if ( !writeString(*container.mValue.mpString) )
				return false;
			break;
		}
		case VuJsonContainer::arrayValue:
		{
			int size = (int)container.mValue.mpArray->size();
			if ( !writeValue(size) )
				return false;
			for ( int index = 0; index < size; index++ )
				if ( !writeContainer((*container.mValue.mpArray)[index]) )
					return false;
			break;
		}
		case VuJsonContainer::objectValue:
		{
			int count = container.numMembers();
			if ( !writeValue(count) )
				return false;
			for ( VuJsonContainer::Object::const_iterator iter = container.mValue.mpObject->begin(); iter != container.mValue.mpObject->end(); iter++ )
			{
				if ( !writeString(iter->second.mKey) )
					return false;
				if ( !writeContainer(iter->second.mValue) )
					return false;
			}
			break;
		}
		case VuJsonContainer::int64Value:
		{
			if ( !writeValue(container.mValue.mInt64) )
				return false;
			break;
		}
		case VuJsonContainer::binaryValue:
		{
			int size = container.mValue.mBinary.mSize;
			if ( !writeValue(size) )
				return false;

			if ( size > mDataRemaining )
				return false;

			VU_MEMCPY(mpDataPtr, mDataRemaining, container.mValue.mBinary.mpData, size);
			mpDataPtr += size;
			mDataRemaining -= size;

			break;
		}
		case VuJsonContainer::force32:
			break;
	}

	return true;
}

//*****************************************************************************
bool VuJsonBinaryWriter::writeString(const std::string &str)
{
	int len = (int)str.length();
	if ( !writeValue(len) )
		return false;

	if ( len > mDataRemaining )
		return false;

	VU_MEMCPY(mpDataPtr, mDataRemaining, str.c_str(), len);
	mpDataPtr += len;
	mDataRemaining -= len;

	return true;
}
