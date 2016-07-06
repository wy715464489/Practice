//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Hashed string class
// 
//*****************************************************************************

#pragma once

#include "VuHash.h"

class VuHashedString
{
public:
	inline VuHashedString() : mSymbol(0) {}
	explicit inline VuHashedString(const VuHashedString &other);
	explicit inline VuHashedString(const char *str);
	explicit inline VuHashedString(const std::string &str);

	operator VUUINT32 () const { return mSymbol; }

	inline bool operator == (const char *str) const;

	inline bool operator == (const VuHashedString &rhs) const	{ return mSymbol == rhs.mSymbol; }
	inline bool operator < (const VuHashedString &rhs) const	{ return mSymbol < rhs.mSymbol; }
	inline bool operator > (const VuHashedString &rhs) const	{ return mSymbol > rhs.mSymbol; }

private:
	VUUINT32	mSymbol;

#ifdef VUDEBUG
	std::string	mRawString;
public:
	const std::string	&getRawString() const { return mRawString; }
private:
#endif
};


VuHashedString::VuHashedString(const VuHashedString &other)
{
	mSymbol = other.mSymbol;

#ifdef VUDEBUG
	mRawString = other.mRawString;
#endif
}

VuHashedString::VuHashedString(const char *str)
{
	mSymbol = VuHash::fnv32String(str);

#ifdef VUDEBUG
	mRawString = str;
#endif
}

VuHashedString::VuHashedString(const std::string &str)
{
	mSymbol = VuHash::fnv32String(str.c_str());

#ifdef VUDEBUG
	mRawString = str;
#endif
}

bool VuHashedString::operator == (const char *str) const
{
	return mSymbol == VuHash::fnv32String(str);
}
