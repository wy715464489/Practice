//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Container for a value/array/object.
// 
//*****************************************************************************

#pragma once


class VuJsonContainer
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

	VuJsonContainer() : mType(nullValue) { mValue.mInt64 = 0; }
	VuJsonContainer(const VuJsonContainer &other) : mType(nullValue) { *this = other; }
	~VuJsonContainer() { clear(); }

	// assignment
	VuJsonContainer				&operator=(const VuJsonContainer &other);

	// utility
	void						clear();

	// array access
	const VuJsonContainer		&operator[](int index) const;
	int							size() const;

	// array manipulation
	VuJsonContainer				&operator[](int index);	// creates new array element if not found
	void						resize(int size);
	VuJsonContainer				&append();
	VuJsonContainer				&insertElement(int index);
	void						removeElement(int index);
	void						removeSwapElement(int index);

	// object access
	const VuJsonContainer		&operator[](const std::string &strKey) const;
	const VuJsonContainer		&operator[](const char *strKey) const;
	int							numMembers() const;
	bool						hasMember(const std::string &strKey) const;
	bool						hasMember(const char *strKey) const;
	void						getMemberKeys(std::vector<std::string> &keys) const;
	void						getMemberKeys(std::vector<const char *> &keys) const;
	const std::string			&getMemberKey(int index) const;

	// object manipulation
	VuJsonContainer				&operator[](const std::string &strKey); // creates new object member if not found
	VuJsonContainer				&operator[](const char *strKey); // creates new object member if not found
	void						removeMember(const std::string &strKey);
	void						removeMember(const char *strKey);

	// equality
	bool						equals(const VuJsonContainer &other) const;
	bool						operator == (const VuJsonContainer &other) const { return equals(other); }
	bool						operator != (const VuJsonContainer &other) const { return !equals(other); }

	// type access
	eType						getType() const		{ return mType; }
	bool						isNull() const		{ return mType == nullValue; }
	bool						isInt() const		{ return mType == intValue; }
	bool						isFloat() const		{ return mType == floatValue; }
	bool						isNumber() const	{ return mType == intValue || mType == floatValue || mType == int64Value; }
	bool						isBool() const		{ return mType == boolValue; }
	bool						isString() const	{ return mType == stringValue; }
	bool						isArray() const		{ return mType == arrayValue; }
	bool						isObject() const	{ return mType == objectValue; }
	bool						isInt64() const		{ return mType == int64Value; }
	bool						isBinary() const	{ return mType == binaryValue; }

	// value access
	int							asInt() const;
	float						asFloat() const;
	bool						asBool() const;
	const std::string			&asString() const;
	const char					*asCString() const;
	VUINT64						asInt64() const;

	bool						getValue(int &iVal) const;
	bool						getValue(float &fVal) const;
	bool						getValue(bool &bVal) const;
	bool						getValue(std::string &strVal) const;
	bool						getValue(VUINT64 &i64Val) const;
	bool						getValue(const void *&pData, int &size) const;

	// value manipulation
	void						putValue(int iVal);
	void						putValue(float fVal);
	void						putValue(bool bVal);
	void						putValue(const char *strVal);
	void						putValue(const std::string &strVal);
	void						putValue(VUINT64 i64Val);
	void						putValue(const void *pData, int size);

	static const VuJsonContainer null;

	// utility
	void						makeArray();
	void						makeObject();

protected:
	struct MapValue;
	typedef std::string String;
	typedef std::vector<VuJsonContainer> Array;
	typedef std::map<VUUINT64, MapValue> Object;
	struct Binary { void *mpData; int mSize; };
	union uValue
	{
		int		mInt;
		float	mFloat;
		bool	mBool;
		String	*mpString;
		Array	*mpArray;
		Object	*mpObject;
		VUINT64	mInt64;
		Binary	mBinary;
	};

	eType	mType;
	uValue	mValue;

	friend class VuJsonBinaryReader;
	friend class VuJsonBinaryWriter;
	friend class VuFastContainer;
};

struct VuJsonContainer::MapValue
{
	std::string		mKey;
	VuJsonContainer	mValue;
};

