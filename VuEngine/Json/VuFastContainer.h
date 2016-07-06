//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Container used to read binary json data directly.
// 
//*****************************************************************************

#pragma once

#include "VuJsonContainer.h"
#include "VuEngine/Util/VuHash.h"

class VuBinaryDataWriter;
class VuBinaryDataReader;


class VuFastContainer
{
public:
	enum eType
	{
		nullValue,
		intValue,
		floatValue,
		boolValue,
		stringValue,
		arrayValue,
		objectValue,
		int64Value,
		binaryValue,

		force32 = 0xffffffff
	};

	VuFastContainer() : mType(nullValue) {}

	// array access
	inline const VuFastContainer	&operator[](int index) const;
	inline int						size() const;

	// object access
	const VuFastContainer			&operator[](const char *strKey) const;
	bool							hasMember(const char *strKey) const;
	inline int						numMembers() const;
	inline const char				*getMemberKey(int index) const;
	inline const VuFastContainer	&getMember(int index) const;

	// type access
	inline eType					getType() const		{ return mType; }
	inline bool						isNull() const		{ return mType == nullValue; }
	inline bool						isInt() const		{ return mType == intValue; }
	inline bool						isFloat() const		{ return mType == floatValue; }
	inline bool						isNumber() const	{ return mType == intValue || mType == floatValue || mType == int64Value; }
	inline bool						isBool() const		{ return mType == boolValue; }
	inline bool						isString() const	{ return mType == stringValue; }
	inline bool						isArray() const		{ return mType == arrayValue; }
	inline bool						isObject() const	{ return mType == objectValue; }
	inline bool						isInt64() const		{ return mType == int64Value; }
	inline bool						isBinary() const	{ return mType == binaryValue; }

	// value access
	inline int						asInt() const;
	inline float					asFloat() const;
	inline bool						asBool() const;
	inline const char				*asCString() const;
	inline VUINT64					asInt64() const;

	inline bool						getValue(int &iVal) const;
	inline bool						getValue(float &fVal) const;
	inline bool						getValue(bool &bVal) const;
	inline bool						getValue(std::string &strVal) const;
	inline bool						getValue(VUINT64 &i64Val) const;
	inline bool						getValue(const void *&pData, int &size) const;

	// serialization
	static void						serialize(const VuJsonContainer &container, VuBinaryDataWriter &writer);
	static const VuFastContainer	*createInPlace(const void *pData);

	static const VuFastContainer null;

protected:
	class StringTable;

	static int			calculateDataSizeRecursiveAndGatherStrings(const VuJsonContainer &container, StringTable &stringTable);
	static void			serializeRecursive(const VuJsonContainer &container, const StringTable &stringTable, VuBinaryDataWriter &writer);

	struct ArrayElement;
	struct ObjectMember;

	eType				mType;
	int					mSize;
	union
	{
		int				mInt;
		float			mFloat;
		VUUINT32		mBool;
		VUUINT32		mStringOffset;
		VUUINT32		mFirstElementOffset;
		VUUINT32		mFirstMemberOffset;
		VUINT64			mInt64;
		VUUINT32		mDataOffset;
	};
};

struct VuFastContainer::ArrayElement
{
	VUUINT32	mPointerOffset;
};

struct VuFastContainer::ObjectMember
{
	VUUINT64	mHashedKey;
	VUUINT32	mKeyOffset;
	VUUINT32	mValueOffset;
};


//*****************************************************************************
inline const VuFastContainer &VuFastContainer::operator[](int index) const
{
	if ( index >= 0 && index < size() )
	{
		VUBYTE *pBase = (VUBYTE *)this;
		ArrayElement *pFirstElement = (ArrayElement *)((VUBYTE *)this + mFirstElementOffset);
		return *(VuFastContainer *)(pBase + pFirstElement[index].mPointerOffset);
	}
	return null;
}

//*****************************************************************************
inline int VuFastContainer::size() const
{
	if ( mType == arrayValue )
		return mSize;
	return 0;
}

//*****************************************************************************
inline int VuFastContainer::numMembers() const
{
	if ( mType == objectValue )
		return mSize;
	return 0;
}

//*****************************************************************************
inline const char *VuFastContainer::getMemberKey(int index) const
{
	if ( index >= 0 && index < numMembers() )
	{
		VUBYTE *pBase = (VUBYTE *)this;
		ObjectMember *pFirstMember = (ObjectMember *)(pBase + mFirstMemberOffset);
		return (const char *)(pBase + pFirstMember[index].mKeyOffset);
	}

	return "";
}

//*****************************************************************************
inline const VuFastContainer &VuFastContainer::getMember(int index) const
{
	if ( index >= 0 && index < numMembers() )
	{
		VUBYTE *pBase = (VUBYTE *)this;
		ObjectMember *pFirstMember = (ObjectMember *)(pBase + mFirstMemberOffset);
		return *(VuFastContainer *)(pBase + pFirstMember[index].mValueOffset);
	}

	return null;
}

//*****************************************************************************
inline int VuFastContainer::asInt() const
{
	if ( mType == intValue )
		return mInt;
	if ( mType == floatValue )
		return (int)mFloat;
	if ( mType == int64Value )
		return (int)mInt64;
	return 0;
}

//*****************************************************************************
inline float VuFastContainer::asFloat() const
{
	if ( mType == intValue )
		return (float)mInt;
	if ( mType == floatValue )
		return mFloat;
	if ( mType == int64Value )
		return (float)mInt64;
	return 0.0f;
}

//*****************************************************************************
inline bool VuFastContainer::asBool() const
{
	if ( mType == boolValue )
		return mBool&0x1;
	return false;
}

//*****************************************************************************
inline const char *VuFastContainer::asCString() const
{
	if ( mType == stringValue )
		return (const char *)((VUBYTE *)this + mStringOffset);
	return "";
}

//*****************************************************************************
inline bool VuFastContainer::getValue(int &iVal) const
{
	if ( mType == intValue )
	{
		iVal = mInt;
		return true;
	}
	if ( mType == floatValue )
	{
		iVal = (int)mFloat;
		return true;
	}
	if ( mType == int64Value )
	{
		iVal = (int)mInt64;
		return true;
	}

	return false;
}

//*****************************************************************************
inline bool VuFastContainer::getValue(float &fVal) const
{
	if ( mType == intValue )
	{
		fVal = (float)mInt;
		return true;
	}
	if ( mType == floatValue )
	{
		fVal = mFloat;
		return true;
	}
	if ( mType == int64Value )
	{
		fVal = (float)mInt64;
		return true;
	}

	return false;
}

//*****************************************************************************
inline bool VuFastContainer::getValue(bool &bVal) const
{
	if ( mType == boolValue )
	{
		bVal = mBool&0x1;
		return true;
	}
	return false;
}

//*****************************************************************************
inline bool VuFastContainer::getValue(std::string &strVal) const
{
	if ( mType == stringValue )
	{
		strVal = (const char *)((VUBYTE *)this + mStringOffset);
		return true;
	}
	return false;
}

//*****************************************************************************
inline bool VuFastContainer::getValue(VUINT64 &i64Val) const
{
	if ( mType == intValue )
	{
		i64Val = mInt;
		return true;
	}
	if ( mType == floatValue )
	{
		i64Val = (VUINT64)mFloat;
		return true;
	}
	if ( mType == int64Value )
	{
		i64Val = mInt64;
		return true;
	}

	return false;
}

//*****************************************************************************
inline bool VuFastContainer::getValue(const void *&pData, int &size) const
{
	if ( mType == binaryValue )
	{
		size = mSize;
		pData = (const void *)((VUBYTE *)this + mDataOffset);
		return true;
	}
	return false;
}
