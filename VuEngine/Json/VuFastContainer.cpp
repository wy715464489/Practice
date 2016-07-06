//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Container used to read binary json data directly.
// 
//*****************************************************************************

#include "VuFastContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


const VuFastContainer VuFastContainer::null;


static VUUINT32 scFastContainerBinaryVersion = 0x1;

class VuFastContainer::StringTable
{
public:
	StringTable() : mBaseOffset(0) {}

	void insert(const std::string &str)
	{
		if ( std::find(mStrings.begin(), mStrings.end(), str) == mStrings.end() )
			mStrings.push_back(str);
	}

	int offset(const std::string &str) const
	{
		int offset = mBaseOffset;
		for ( Strings::const_iterator iter = mStrings.begin(); iter != mStrings.end(); iter++ )
		{
			if ( *iter == str )
				return offset;
			offset += (int)iter->length() + 1;
		}
		VUASSERT(0, "VuFastContainer unable to find string");
		return 0;
	}

	int calcDataSize()
	{
		int dataSize = 0;
		for ( const std::string &str : mStrings )
			dataSize += (int)str.length() + 1;
		return dataSize;
	}

	void serialize(VuBinaryDataWriter &writer)
	{
		for ( Strings::const_iterator iter = mStrings.begin(); iter != mStrings.end(); iter++ )
			writer.writeString(*iter);
	}

	typedef std::deque<std::string> Strings;
	Strings		mStrings;
	int			mBaseOffset;
};


//*****************************************************************************
const VuFastContainer &VuFastContainer::operator[](const char *strKey) const
{
	if ( mType == objectValue )
	{
		VUUINT64 hashedKey = VuHash::fnv64String(strKey);

		VUBYTE *pBase = (VUBYTE *)this;
		ObjectMember *pFirstMember = (ObjectMember *)(pBase + mFirstMemberOffset);

		// binary search
		int start = 0, end = mSize;
		while ( start < end )
		{
			int middle = (start + end)>>1;
			ObjectMember &member = pFirstMember[middle];
			if ( hashedKey < member.mHashedKey )
				end = middle;
			else if ( hashedKey > member.mHashedKey )
				start = middle + 1;
			else
				return *(VuFastContainer *)(pBase + member.mValueOffset);
		}
	}

	return null;
}

//*****************************************************************************
bool VuFastContainer::hasMember(const char *strKey) const
{
	if ( mType == objectValue )
	{
		VUUINT64 hashedKey = VuHash::fnv64String(strKey);

		VUBYTE *pBase = (VUBYTE *)this;
		ObjectMember *pFirstMember = (ObjectMember *)(pBase + mFirstMemberOffset);

		// binary search
		int start = 0, end = mSize;
		while ( start < end )
		{
			int middle = (start + end)>>1;
			ObjectMember &member = pFirstMember[middle];
			if ( hashedKey < member.mHashedKey )
				end = middle;
			else if ( hashedKey > member.mHashedKey )
				start = middle + 1;
			else
				return true;
		}
	}
	return false;
}

//*****************************************************************************
void VuFastContainer::serialize(const VuJsonContainer &container, VuBinaryDataWriter &writer)
{
	StringTable stringTable;

	int baseSize = writer.size();
	int containerSize = calculateDataSizeRecursiveAndGatherStrings(container, stringTable);
	int stringTableSize = stringTable.calcDataSize();
	writer.reserve(8 + containerSize + stringTableSize);

	stringTable.mBaseOffset = 8 + containerSize;

	writer.writeValue(scFastContainerBinaryVersion);
	writer.writeValue(containerSize);
	serializeRecursive(container, stringTable, writer);
	stringTable.serialize(writer);

	VUASSERT(writer.size() == baseSize + 8 + containerSize + stringTableSize, "VuFastContainer::serialize() something went wrong!");
}

//*****************************************************************************
const VuFastContainer *VuFastContainer::createInPlace(const void *pData)
{
	VUASSERT(((VUUINT64)pData&0x3) == 0, "VuFastContainer::createInPlace() alignment issue");

	int *p = (int *)pData;
	int version = *p++;
	int containerDataSize = *p++;

	VUASSERT(version == scFastContainerBinaryVersion, "VuFastContainer version mismatch");

	VuFastContainer *pFastContainer = (VuFastContainer *)(p);

	return pFastContainer;
}

//*****************************************************************************
int VuFastContainer::calculateDataSizeRecursiveAndGatherStrings(const VuJsonContainer &container, StringTable &stringTable)
{
	int dataSize = 16;

	switch ( container.mType )
	{
		case VuJsonContainer::stringValue:
		{
			stringTable.insert(*container.mValue.mpString);
			break;
		}
		case VuJsonContainer::arrayValue:
		{
			int size = (int)container.mValue.mpArray->size();
			dataSize += size*4;
			for ( int index = 0; index < size; index++ )
				dataSize += calculateDataSizeRecursiveAndGatherStrings((*container.mValue.mpArray)[index], stringTable);
			break;
		}
		case VuJsonContainer::objectValue:
		{
			int count = container.numMembers();
			dataSize += count*16;
			for ( VuJsonContainer::Object::const_iterator iter = container.mValue.mpObject->begin(); iter != container.mValue.mpObject->end(); iter++ )
			{
				stringTable.insert(iter->second.mKey);
				dataSize += calculateDataSizeRecursiveAndGatherStrings(iter->second.mValue, stringTable);
			}
			break;
		}
		case VuJsonContainer::binaryValue:
		{
			dataSize += container.mValue.mBinary.mSize;
			break;
		}
		default:
		{
			break;
		}
	}

	return dataSize;
}

//*****************************************************************************
void VuFastContainer::serializeRecursive(const VuJsonContainer &container, const StringTable &stringTable, VuBinaryDataWriter &writer)
{
	VUUINT32 baseOffset = writer.size();

	writer.writeValue(container.mType);

	switch ( container.mType )
	{
		case VuJsonContainer::nullValue:
		{
			writer.writePad(12);
			break;
		}
		case VuJsonContainer::intValue:
		{
			writer.writePad(4); // size
			writer.writeValue(container.mValue.mInt);
			writer.writePad(4);
			break;
		}
		case VuJsonContainer::floatValue:
		{
			writer.writePad(4); // size
			writer.writeValue(container.mValue.mFloat);
			writer.writePad(4);
			break;
		}
		case VuJsonContainer::boolValue:
		{
			writer.writePad(4); // size
			VUUINT32 value = container.mValue.mBool;
			writer.writeValue(value);
			writer.writePad(4);
			break;
		}
		case VuJsonContainer::stringValue:
		{
			writer.writePad(4); // size

			VUUINT32 stringOffset = stringTable.offset(*container.mValue.mpString) - baseOffset;
			writer.writeValue(stringOffset);

			writer.writePad(4);
			break;
		}
		case VuJsonContainer::arrayValue:
		{
			int size = (int)container.mValue.mpArray->size();
			writer.writeValue(size);

			VUUINT32 firstElementOffset = writer.size() + 8 - baseOffset;
			writer.writeValue(firstElementOffset);

			writer.writePad(4);

			ArrayElement *pElements = (ArrayElement *)writer.allocate(size*4);
			for ( int index = 0; index < size; index++ )
			{
				pElements[index].mPointerOffset = writer.size() - baseOffset;
				serializeRecursive((*container.mValue.mpArray)[index], stringTable, writer);
			}
			break;
		}
		case VuJsonContainer::objectValue:
		{
			int count = container.numMembers();
			writer.writeValue(count);

			VUUINT32 firstMemberOffset = writer.size() + 8 - baseOffset;
			writer.writeValue(firstMemberOffset);

			writer.writePad(4);

			ObjectMember *pMembers = (ObjectMember *)writer.allocate(count*16);
			int index = 0;
			for ( VuJsonContainer::Object::const_iterator iter = container.mValue.mpObject->begin(); iter != container.mValue.mpObject->end(); iter++ )
			{
				pMembers[index].mHashedKey = iter->first;
				pMembers[index].mKeyOffset = stringTable.offset(iter->second.mKey) - baseOffset;
				pMembers[index].mValueOffset = writer.size() - baseOffset;
				serializeRecursive(iter->second.mValue, stringTable, writer);
				index++;
			}
			break;
		}
		case VuJsonContainer::int64Value:
		{
			writer.writePad(4); // size
			writer.writeValue(container.mValue.mInt64);
			break;
		}
		case VuJsonContainer::binaryValue:
		{
			int size = container.mValue.mBinary.mSize;
			writer.writeValue(size);

			VUUINT32 dataOffset = writer.size() + 8 - baseOffset;
			writer.writeValue(dataOffset);

			writer.writePad(4);

			writer.writeData(container.mValue.mBinary.mpData, size);
			break;
		}
		default:
		{
			VUASSERT(0, "VuFastContainer::serialize() unknown container type!")
			break;
		}
	}
}
