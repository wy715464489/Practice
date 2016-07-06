//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Container for a value/array/object.
// 
//*****************************************************************************

#include "VuJsonContainer.h"
#include "VuEngine/Util/VuHash.h"


const VuJsonContainer VuJsonContainer::null;


//*****************************************************************************
VuJsonContainer &VuJsonContainer::operator=(const VuJsonContainer &other)
{
	clear();

	mType = other.mType;
	switch ( other.mType )
	{
		case nullValue:
		{
			break;
		}
		case intValue:
		{
			mValue.mInt = other.mValue.mInt;
			break;
		}
		case floatValue:
		{
			mValue.mFloat = other.mValue.mFloat;
			break;
		}
		case boolValue:
		{
			mValue.mBool = other.mValue.mBool;
			break;
		}
		case stringValue:
		{
			mValue.mpString = new String;
			*mValue.mpString = *other.mValue.mpString;
			break;
		}
		case arrayValue:
		{
			mValue.mpArray = new Array;
			*mValue.mpArray = *other.mValue.mpArray;
			break;
		}
		case objectValue:
		{
			mValue.mpObject = new Object;
			*mValue.mpObject = *other.mValue.mpObject;
			break;
		}
		case int64Value:
		{
			mValue.mInt64 = other.mValue.mInt64;
			break;
		}
		case binaryValue:
		{
			int size = other.mValue.mBinary.mSize;
			mValue.mBinary.mSize = size;
			mValue.mBinary.mpData = malloc(size);
			VU_MEMCPY(mValue.mBinary.mpData, size, other.mValue.mBinary.mpData, size);
			break;
		}
		case force32:
			break;
	}

	return *this;
}

//*****************************************************************************
void VuJsonContainer::clear()
{
	switch ( mType )
	{
		case stringValue:
		{
			delete mValue.mpString;
			break;
		}
		case arrayValue:
		{
			delete mValue.mpArray;
			break;
		}
		case objectValue:
		{
			delete mValue.mpObject;
			break;
		}
		case binaryValue:
		{
			free(mValue.mBinary.mpData);
			break;
		}
		default:
		{
			break;
		}
	}

	mType = nullValue;
	mValue.mInt64 = 0LL;
}

//*****************************************************************************
const VuJsonContainer &VuJsonContainer::operator[](int index) const
{
	if ( mType == arrayValue )
		if ( index >= 0 && index < (int)mValue.mpArray->size() )
			return (*mValue.mpArray)[index];

	return null;
}

//*****************************************************************************
int VuJsonContainer::size() const
{
	return mType == arrayValue ? (int)mValue.mpArray->size() : 0;
}

//*****************************************************************************
VuJsonContainer &VuJsonContainer::operator[](int index)
{
	makeArray();

	if ( index >= (int)mValue.mpArray->size() )
		mValue.mpArray->resize(index + 1);

	return (*mValue.mpArray)[index];
}

//*****************************************************************************
void VuJsonContainer::resize(int size)
{
	makeArray();

	mValue.mpArray->resize(size);
}

//*****************************************************************************
VuJsonContainer &VuJsonContainer::append()
{
	makeArray();
	return (*this)[size()];
}

//*****************************************************************************
VuJsonContainer &VuJsonContainer::insertElement(int index)
{
	if ( index < size() )
	{
		int oldSize = size();
		mValue.mpArray->resize(oldSize + 1);
		for ( int i = oldSize; i > index; i-- )
			(*mValue.mpArray)[i] = (*mValue.mpArray)[i-1];
		(*mValue.mpArray)[index].clear();
	}

	return (*this)[index];
}

//*****************************************************************************
void VuJsonContainer::removeElement(int index)
{
	int arraySize = size();
	if ( index >= 0 && index < arraySize )
	{
		for ( int i = index + 1; i < arraySize; i++ )
			(*mValue.mpArray)[i-1] = (*mValue.mpArray)[i];
		mValue.mpArray->resize(arraySize - 1);
	}
}

//*****************************************************************************
void VuJsonContainer::removeSwapElement(int index)
{
	int arraySize = size();
	if ( index >= 0 && index < arraySize )
	{
		(*mValue.mpArray)[index] = (*mValue.mpArray)[arraySize - 1];
		mValue.mpArray->resize(arraySize - 1);
	}
}

//*****************************************************************************
const VuJsonContainer &VuJsonContainer::operator[](const std::string &strKey) const
{
	if ( mType == objectValue )
	{
		VUUINT64 key = VuHash::fnv64String(strKey.c_str());
		Object::iterator iter = mValue.mpObject->find(key);
		if ( iter != mValue.mpObject->end() )
			return iter->second.mValue;
	}

	return null;
}

//*****************************************************************************
const VuJsonContainer &VuJsonContainer::operator[](const char *strKey) const
{
	if ( mType == objectValue )
	{
		VUUINT64 key = VuHash::fnv64String(strKey);
		Object::iterator iter = mValue.mpObject->find(key);
		if ( iter != mValue.mpObject->end() )
			return iter->second.mValue;
	}

	return null;
}

//*****************************************************************************
int VuJsonContainer::numMembers() const
{
	return mType == objectValue ? (int)mValue.mpObject->size() : 0;
}

//*****************************************************************************
bool VuJsonContainer::hasMember(const std::string &strKey) const
{
	if ( mType == objectValue )
	{
		VUUINT64 key = VuHash::fnv64String(strKey.c_str());
		return mValue.mpObject->find(key) != mValue.mpObject->end();
	}
	return false;
}

//*****************************************************************************
bool VuJsonContainer::hasMember(const char *strKey) const
{
	if ( mType == objectValue )
	{
		VUUINT64 key = VuHash::fnv64String(strKey);
		return mValue.mpObject->find(key) != mValue.mpObject->end();
	}
	return false;
}

//*****************************************************************************
void VuJsonContainer::getMemberKeys(std::vector<std::string> &keys) const
{
	keys.clear();

	if ( mType == objectValue )
		for ( Object::iterator iter = mValue.mpObject->begin(); iter != mValue.mpObject->end(); iter++ )
			keys.push_back(iter->second.mKey);

	std::sort(keys.begin(), keys.end());
}

//*****************************************************************************
bool CompareKeys(const char *key0, const char *key1)
{
	return strcmp(key0, key1) < 0;
}
void VuJsonContainer::getMemberKeys(std::vector<const char *> &keys) const
{
	keys.clear();

	if ( mType == objectValue )
		for ( Object::iterator iter = mValue.mpObject->begin(); iter != mValue.mpObject->end(); iter++ )
			keys.push_back(iter->second.mKey.c_str());

	std::sort(keys.begin(), keys.end(), CompareKeys);
}

//*****************************************************************************
const std::string &VuJsonContainer::getMemberKey(int index) const
{
	if ( index >= 0 && index < numMembers() )
	{
		Object::iterator iter = mValue.mpObject->begin();
		for ( int i = 0; i < index; i++ )
			iter++;
		return iter->second.mKey;
	}

	static std::string sEmtpy;
	return sEmtpy;
}

//*****************************************************************************
VuJsonContainer &VuJsonContainer::operator[](const std::string &strKey)
{
	makeObject();
	VUUINT64 key = VuHash::fnv64String(strKey.c_str());
	MapValue &value = (*mValue.mpObject)[key];
	if ( value.mKey.empty() )
		value.mKey = strKey;
	else
		VUASSERT(value.mKey.compare(strKey) == 0, "VuJsonContainer key collision");
	return value.mValue;
}

//*****************************************************************************
VuJsonContainer &VuJsonContainer::operator[](const char *strKey)
{
	makeObject();
	VUUINT64 key = VuHash::fnv64String(strKey);
	MapValue &value = (*mValue.mpObject)[key];
	if ( value.mKey.empty() )
		value.mKey = strKey;
	else
		VUASSERT(value.mKey.compare(strKey) == 0, "VuJsonContainer key collision");
	return value.mValue;
}

//*****************************************************************************
void VuJsonContainer::removeMember(const std::string &strKey)
{
	if ( mType == objectValue )
	{
		VUUINT64 key = VuHash::fnv64String(strKey.c_str());
		mValue.mpObject->erase(key);
	}
}

//*****************************************************************************
void VuJsonContainer::removeMember(const char *strKey)
{
	if ( mType == objectValue )
	{
		mValue.mpObject->erase(VuHash::fnv64String(strKey));
	}
}

//*****************************************************************************
bool VuJsonContainer::equals(const VuJsonContainer &other) const
{
	if ( getType() != other.getType() )
		return false;

	switch ( getType() )
	{
		case nullValue:
		{
			return true;
		}
		case intValue:
		{
			return asInt() == other.asInt();
		}
		case floatValue:
		{
			return asFloat() == other.asFloat();
		}
		case boolValue:
		{
			return asBool() == other.asBool();
		}
		case stringValue:
		{
			return asString() == other.asString();
		}
		case arrayValue:
		{
			if ( size() != other.size() )
				return false;

			for ( int i = 0; i < size(); i++ )
				if ( (*this)[i] != other[i] )
					return false;

			return true;
		}
		case objectValue:
		{
			if ( numMembers() != other.numMembers() )
				return false;

			for ( int i = 0; i < numMembers(); i++ )
			{
				const std::string &key = getMemberKey(i);
				if ( key != other.getMemberKey(i) )
					return false;

				if ( (*this)[key] != other[key] )
					return false;
			}

			return true;
		}
		case int64Value:
		{
			return asInt64() == other.asInt64();
		}
		case binaryValue:
		{
			if ( mValue.mBinary.mSize != other.mValue.mBinary.mSize )
				return false;

			return memcmp(mValue.mBinary.mpData, other.mValue.mBinary.mpData, mValue.mBinary.mSize) == 0;
		}
		case force32:
			break;
	}

	// A new type was added that I don't know about.
	return false;
}

//*****************************************************************************
int VuJsonContainer::asInt() const
{
	if ( mType == intValue )
		return mValue.mInt;
	if ( mType == floatValue )
		return (int)mValue.mFloat;
	if ( mType == int64Value )
		return (int)mValue.mInt64;

	return 0;
}

//*****************************************************************************
float VuJsonContainer::asFloat() const
{
	if ( mType == intValue )
		return (float)mValue.mInt;
	if ( mType == floatValue )
		return mValue.mFloat;
	if ( mType == int64Value )
		return (float)mValue.mInt64;

	return 0.0f;
}

//*****************************************************************************
bool VuJsonContainer::asBool() const
{
	return mType == boolValue ? mValue.mBool : false;
}

//*****************************************************************************
const std::string &VuJsonContainer::asString() const
{
	static std::string sEmtpy;
	return mType == stringValue ? *mValue.mpString : sEmtpy;
}

//*****************************************************************************
const char *VuJsonContainer::asCString() const
{
	return mType == stringValue ? mValue.mpString->c_str() : "";
}

//*****************************************************************************
VUINT64 VuJsonContainer::asInt64() const
{
	if ( mType == intValue )
		return (VUINT64)mValue.mInt;
	if ( mType == floatValue )
		return (VUINT64)mValue.mFloat;
	if ( mType == int64Value )
		return mValue.mInt64;

	return 0LL;
}

//*****************************************************************************
bool VuJsonContainer::getValue(int &iVal) const
{
	if ( mType == intValue )
	{
		iVal = mValue.mInt;
		return true;
	}
	if ( mType == floatValue )
	{
		iVal = (int)mValue.mFloat;
		return true;
	}
	if ( mType == int64Value )
	{
		iVal = (int)mValue.mInt64;
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuJsonContainer::getValue(float &fVal) const
{
	if ( mType == intValue )
	{
		fVal = (float)mValue.mInt;
		return true;
	}
	if ( mType == floatValue )
	{
		fVal = mValue.mFloat;
		return true;
	}
	if ( mType == int64Value )
	{
		fVal = (float)mValue.mInt64;
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuJsonContainer::getValue(bool &bVal) const
{
	if ( mType == boolValue )
	{
		bVal = mValue.mBool;
		return true;
	}
	return false;
}

//*****************************************************************************
bool VuJsonContainer::getValue(std::string &strVal) const
{
	if ( mType == stringValue )
	{
		strVal = *mValue.mpString;
		return true;
	}
	return false;
}

//*****************************************************************************
bool VuJsonContainer::getValue(VUINT64 &i64Val) const
{
	if ( mType == intValue )
	{
		i64Val = mValue.mInt;
		return true;
	}
	if ( mType == floatValue )
	{
		i64Val = (VUINT64)mValue.mFloat;
		return true;
	}
	if ( mType == int64Value )
	{
		i64Val = mValue.mInt64;
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuJsonContainer::getValue(const void *&pData, int &size) const
{
	if ( mType == binaryValue )
	{
		size = mValue.mBinary.mSize;
		pData = mValue.mBinary.mpData;
		return true;
	}
	return false;
}

//*****************************************************************************
void VuJsonContainer::putValue(int iVal)
{
	clear();
	mType = intValue;
	mValue.mInt = iVal;
}

//*****************************************************************************
void VuJsonContainer::putValue(float fVal)
{
	clear();
	mType = floatValue;
	mValue.mFloat = fVal;
}

//*****************************************************************************
void VuJsonContainer::putValue(bool bVal)
{
	clear();
	mType = boolValue;
	mValue.mBool = bVal;
}

//*****************************************************************************
void VuJsonContainer::putValue(const char *strVal)
{
	clear();
	mType = stringValue;
	mValue.mpString = new String;
	*mValue.mpString = strVal;
}

//*****************************************************************************
void VuJsonContainer::putValue(const std::string &strVal)
{
	putValue(strVal.c_str());
}

//*****************************************************************************
void VuJsonContainer::putValue(VUINT64 i64Val)
{
	clear();
	mType = int64Value;
	mValue.mInt64 = i64Val;
}

//*****************************************************************************
void VuJsonContainer::putValue(const void *pData, int size)
{
	clear();
	mType = binaryValue;
	mValue.mBinary.mSize = size;
	mValue.mBinary.mpData = malloc(size);
	VU_MEMCPY(mValue.mBinary.mpData, size, pData, size);
}

//*****************************************************************************
void VuJsonContainer::makeArray()
{
	if ( mType != arrayValue )
	{
		clear();
		mType = arrayValue;
		mValue.mpArray = new Array;
	}
}

//*****************************************************************************
void VuJsonContainer::makeObject()
{
	if ( mType != objectValue )
	{
		clear();
		mType = objectValue;
		mValue.mpObject = new Object;
	}
}
