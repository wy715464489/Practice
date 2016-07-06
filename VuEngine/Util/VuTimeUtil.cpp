//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Time utility
// 
//*****************************************************************************

#include "VuTimeUtil.h"
#include "VuEngine/Math/VuMath.h"


//*****************************************************************************
void VuTimeUtil::getLocalTime(VuTimeStruct &timeStruct)
{
	std::time_t rawTime;
	std::time(&rawTime);

	std::tm localTime;
	VU_LOCALTIME(&rawTime, &localTime);

	timeStruct.mYear = localTime.tm_year + 1900;
	timeStruct.mMonth = localTime.tm_mon + 1;
	timeStruct.mDay = localTime.tm_mday;
	timeStruct.mHour = localTime.tm_hour;
	timeStruct.mMinute = localTime.tm_min;
	timeStruct.mSecond = localTime.tm_sec;
}

//*****************************************************************************
void VuTimeUtil::getCompileTime(VuTimeStruct &timeStruct)
{
	memset(&timeStruct, 0, sizeof(timeStruct));

	const char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	char month[16] = "";

	VU_SSCANF(__TIME__, "%d:%d:%d", &timeStruct.mHour, &timeStruct.mMinute, &timeStruct.mSecond);

#if VU_SAFE_STDIO
	VU_SSCANF(__DATE__, "%s %d %d", month, sizeof(month), &timeStruct.mDay, &timeStruct.mYear);
#else
	VU_SSCANF(__DATE__, "%s %d %d", month, &timeStruct.mDay, &timeStruct.mYear);
#endif

	for ( int i = 0; i < 12; i++ )
		if ( strcmp(month, months[i]) == 0 )
			timeStruct.mMonth = i + 1;
}

//*****************************************************************************
VUINT64 VuTimeUtil::diffTime(const VuTimeStruct &tsa, const VuTimeStruct &tsb)
{
	std::tm tma;
	memset(&tma, 0, sizeof(tma));
	tma.tm_year = tsa.mYear - 1900;
	tma.tm_mon = tsa.mMonth - 1;
	tma.tm_mday = tsa.mDay;
	tma.tm_hour = tsa.mHour;
	tma.tm_min = tsa.mMinute;
	tma.tm_sec = tsa.mSecond;

	std::tm tmb;
	memset(&tmb, 0, sizeof(tmb));
	tmb.tm_year = tsb.mYear - 1900;
	tmb.tm_mon = tsb.mMonth - 1;
	tmb.tm_mday = tsb.mDay;
	tmb.tm_hour = tsb.mHour;
	tmb.tm_min = tsb.mMinute;
	tmb.tm_sec = tsb.mSecond;

	double seconds = std::difftime(std::mktime(&tma), std::mktime(&tmb));

	return VUINT64(seconds + 0.5);
}

//*****************************************************************************
int VuTimeUtil::calcDaysInMonth(int year, int month)
{
	static const int sMonthDays[2][12] =
	{
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	};

	VUASSERT(month >= 0 && month <= 11, "calcDaysInMonth() invalid month");

	int isLeapYear = year > 0 && !(year % 4) && (year % 100 || !(year % 400));
	
	return sMonthDays[isLeapYear][month];
}

//*****************************************************************************
int VuTimeUtil::calcDaysSince2000(int year, int month, int day)
{
	std::tm newYear2000;
	memset(&newYear2000, 0, sizeof(newYear2000));
	newYear2000.tm_mday = 1;
	newYear2000.tm_mon = 0;
	newYear2000.tm_year = 2000 - 1900;

	std::tm today;
	memset(&today, 0, sizeof(today));
	today.tm_mday = day;
	today.tm_mon = month;
	today.tm_year = year - 1900;

	double seconds = std::difftime(std::mktime(&today), std::mktime(&newYear2000));
	double days = seconds/(24.0f*3600.0f);

	return VuRound((float)days);
}

//*****************************************************************************
int VuTimeUtil::calcDaysSince2000()
{
	std::time_t rawTime;
	std::time(&rawTime);

	std::tm localTime;
	VU_LOCALTIME(&rawTime, &localTime);

	return VuTimeUtil::calcDaysSince2000(localTime.tm_year + 1900, localTime.tm_mon, localTime.tm_mday);
}

//*****************************************************************************
VUINT64 VuTimeUtil::calcSecondsSince2000()
{
	std::time_t rawTime;
	std::time(&rawTime);

	std::tm localTime;
	VU_LOCALTIME(&rawTime, &localTime);

	VUINT64 daysSince2000 = VuTimeUtil::calcDaysSince2000(localTime.tm_year + 1900, localTime.tm_mon, localTime.tm_mday);
	VUINT64 secondsSince2000 = daysSince2000*24*3600 + localTime.tm_hour*3600 + localTime.tm_min*60 + localTime.tm_sec;

	return secondsSince2000;
}