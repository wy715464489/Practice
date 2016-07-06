//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Json binary reader inline functionality.
// 
//*****************************************************************************

#include "VuEngine/Util/VuEndianUtil.h"


template <class T>
bool VuJsonBinaryReader::readValue(T &val)
{
	if ( sizeof(T) > mDataRemaining )
		return error("Read error");

	VuEndianUtil::swapIfLittle(*(const T *)mpDataPtr, val);

	mpDataPtr += sizeof(T);
	mDataRemaining -= sizeof(T);

	return true;
}
