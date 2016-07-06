//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Guid class
// 
//*****************************************************************************

#include "VuGuid.h"
#include "VuHash.h"
#include "VuTimeUtil.h"


//*****************************************************************************
std::string VuGuid::toString() const
{
	char str[48];
	VU_SPRINTF(str, sizeof(str), "%8X-%4X-%4X-%4X-%4X%8X", mValue0, mValue1>>4, mValue1&0xffff, mValue2>>4, mValue2&0xffff, mValue3);
	return str;
}

//*****************************************************************************
VUUINT32 VuGuid::hash() const
{
	return VuHash::fnv32(this, sizeof(*this));
}

//*****************************************************************************
VuGuid VuGuid::create()
{
	VuTimeUtil::VuTimeStruct localTime;
	VuTimeUtil::getLocalTime(localTime);

	VUUINT64 timeCounter = 0;;
	timeCounter = localTime.mYear - 2000;
	timeCounter = timeCounter*12 + localTime.mMonth;
	timeCounter = timeCounter*31 + localTime.mDay;
	timeCounter = timeCounter*24 + localTime.mHour;
	timeCounter = timeCounter*3600 + localTime.mSecond;
	VUUINT64 perfCounter = VuSys::IF()->getPerfCounter();

	VuGuid guid;
	guid.mValue0 = timeCounter >> 32;
	guid.mValue1 = timeCounter & 0xffffffff;
	guid.mValue2 = perfCounter >> 32;
	guid.mValue3 = perfCounter & 0xffffffff;

	return guid;
}
