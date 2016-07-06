//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Json binary writer inline functionality.
// 
//*****************************************************************************

#include "VuEngine/Util/VuEndianUtil.h"


template <class T>
bool VuJsonBinaryWriter::writeValue(const T &val)
{
	if ( sizeof(T) > mDataRemaining )
		return false;

	VuEndianUtil::swapIfLittle(val, *(T *)mpDataPtr);

	mpDataPtr += sizeof(T);
	mDataRemaining -= sizeof(T);

	return true;
}
