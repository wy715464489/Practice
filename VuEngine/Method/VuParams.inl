//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuParams inline functionality
// 
//*****************************************************************************


//*****************************************************************************
// gets a basic type of value from a parameter list
//*****************************************************************************
template<class T>
const T VuParams::VuAccessor::getBasicValue(VuParams::eType type, const T &defaultValue)
{
	T value = defaultValue;

	if ( verifyNextType(type) )
	{
		mpDataRemaining += sizeof(type);
		mSizeRemaining -= sizeof(type);

		VU_MEMCPY(&value, sizeof(value), mpDataRemaining, sizeof(value));
		mpDataRemaining += sizeof(value);
		mSizeRemaining -= sizeof(value);
	}

	return value;
}

//*****************************************************************************
inline bool VuParams::operator == (const VuParams &other) const
{
	if ( mSize == other.mSize )
		if ( memcmp(mData, other.mData, mSize) == 0 )
			return true;

	return false;
}
