//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Time format utility.
// 
//*****************************************************************************

#include <ctype.h>

#include "VuStringUtil.h"
#include "VuEngine/Math/VuMath.h"


//*****************************************************************************
const char *VuStringUtil::timeFormatMilliseconds(eTimeFormat format, VUINT ms, char *str, VUUINT maxLen)
{
	VUASSERT(str, "NULL string passed to VuStringUtil::timeFormat");

	char *dst = str;
	if ( ms < 0 )
	{
		*dst++ = '-';
		maxLen--;
		ms = -ms;
	}

	int thousandths = ms;
	int hundredths = thousandths / 10;
	int secs = hundredths / 100;
	int mins = secs / 60;
	int hours = mins / 60;

	switch(format)
	{
	case HH_MM:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d:%02d", hours, mins % 60);
		break;

	case HH_MM_SS:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d:%02d:%02d", hours, mins % 60, secs % 60);
		break;

	case MM_SS:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d:%02d", mins, secs % 60);
		break;

	case MM_SS_HH:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d:%02d.%02d", mins, secs % 60, hundredths % 100);
		break;

	case MM_SS_TTT:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d:%02d.%03d", mins, secs % 60, thousandths % 1000);
		break;

	case SS:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d", secs);
		break;

	case SS_HH:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d.%02d", secs, hundredths % 100);
		break;

	case SS_TTT:
		VU_SNPRINTF(dst, maxLen, maxLen, "%d.%03d", secs, thousandths % 1000);
		break;
	}

	return str;
}

//*****************************************************************************
const char *VuStringUtil::timeFormatSeconds(eTimeFormat format, float seconds, char *str, VUUINT maxLen)
{
	return timeFormatMilliseconds(format, VuTruncate(seconds*1000.0f), str, maxLen);
}

//*****************************************************************************
const char *VuStringUtil::timeFormatSeconds(eTimeFormat format, int seconds, char *str, VUUINT maxLen)
{
	return timeFormatMilliseconds(format, seconds*1000, str, maxLen);
}

//*****************************************************************************
const char *VuStringUtil::dateFormat(eDateFormat format, int year, int month, int day, char *str, VUUINT maxLen)
{
	switch(format)
	{
	case DD_MM_YYYY:
		VU_SNPRINTF(str, maxLen, maxLen, "%d/%d/%d", day, month, year);
		break;

	case MM_DD_YYYY:
		VU_SNPRINTF(str, maxLen, maxLen, "%d/%d/%d", month, day, year);
		break;

	case YYYY_MM_DD:
		VU_SNPRINTF(str, maxLen, maxLen, "%d/%d/%d", year, month, day);
		break;
	}

	return str;
}

//*****************************************************************************
const char *VuStringUtil::currencyFormat(eCurrencyFormat format, float amount, char *str, VUUINT maxLen)
{
	int cents = VuRound(amount*100);
	int dollars = cents/100;
	cents = VuAbs(cents)%100;

	return currencyFormat(format, dollars, cents, str, maxLen);
}

//*****************************************************************************
const char *VuStringUtil::currencyFormat(eCurrencyFormat format, int dollars, int cents, char *str, VUUINT maxLen)
{
	VUASSERT(str, "NULL string passed to VuStringUtil::currencyFormat");

	char *dst = str;
	if ( dollars < 0 )
	{
		*dst++ = '-';
		maxLen--;
		dollars = -dollars;
	}

	VUUINT32 thousands = dollars/1000;
	VUUINT32 millions = thousands/1000;

	
	switch(format)
	{
	case USD_CC:
		if ( dollars < 1000 )
			VU_SNPRINTF(dst, maxLen, maxLen, "$%d.%02d", dollars, cents % 100);
		else if ( dollars < 1000000 )
			VU_SNPRINTF(dst, maxLen, maxLen, "$%d,%03d.%02d", thousands, dollars % 1000, cents % 100);
		else
			VU_SNPRINTF(dst, maxLen, maxLen, "$%d,%03d,%03d.%02d", millions, thousands % 1000, dollars % 1000, cents % 100);
		break;

	case USD:
		if ( dollars < 1000 )
			VU_SNPRINTF(dst, maxLen, maxLen, "$%d", dollars);
		else if ( dollars < 1000000 )
			VU_SNPRINTF(dst, maxLen, maxLen, "$%d,%03d", thousands, dollars % 1000);
		else
			VU_SNPRINTF(dst, maxLen, maxLen, "$%d,%03d,%03d", millions, thousands % 1000, dollars % 1000);
		break;
	}

	return str;
}

//*****************************************************************************
const char *VuStringUtil::floatFormat(eFloatFormat format, float value, char *str, VUUINT maxLen)
{
	char *dst = str;
	if ( value < 0 )
	{
		*dst++ = '-';
		maxLen--;
		value = -value;
	}

	integerFormat(VuTruncate(value), dst, maxLen);

	int len = (int)strlen(dst);
	dst += len;
	maxLen -= len;

	if ( format == D_T )
		VU_SPRINTF(dst, maxLen, ".%01d", VuTruncate((value - VuFloor(value))*10));
	else if ( format == D_HH )
		VU_SPRINTF(dst, maxLen, ".%02d", VuTruncate((value - VuFloor(value))*100));
	else if ( format == D_TTT )
		VU_SPRINTF(dst, maxLen, ".%03d", VuTruncate((value - VuFloor(value))*1000));

	return str;
}

//*****************************************************************************
const char *VuStringUtil::integerFormat(int value, char *str, VUUINT maxLen)
{
	char *dst = str;
	if ( value < 0 )
	{
		*dst++ = '-';
		maxLen--;
		value = -value;
	}

	int thousands = value/1000;
	int millions = thousands/1000;
	int billions = millions/1000;

	if ( billions )
		VU_SPRINTF(str, maxLen, "%d,%03d,%03d,%03d", billions, millions%1000, thousands%1000, value%1000);
	else if ( millions )
		VU_SPRINTF(str, maxLen, "%d,%03d,%03d", millions, thousands%1000, value%1000);
	else if ( thousands )
		VU_SPRINTF(str, maxLen, "%d,%03d", thousands, value%1000);
	else
		VU_SPRINTF(str, maxLen, "%d", value);

	return str;
}

//*****************************************************************************
const char *VuStringUtil::buildNumberFormat(int buildNumber, char *str, VUUINT maxLen)
{
	const char *strMonthNames[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	char strMonth[16] = "";
	int day = 0, month = 0, year = 0;
#if VU_SAFE_STDIO
	VU_SSCANF(__DATE__, "%s %d %d", strMonth, sizeof(strMonth), &day, &year);
#else
	VU_SSCANF(__DATE__, "%s %d %d", strMonth, &day, &year);
#endif
	for ( int i = 0; i < 12; i++ )
		if ( strcmp(strMonth, strMonthNames[i]) == 0 )
			month = i + 1;

	VU_SPRINTF(str, maxLen, "%02d.%02d.%02d.%04d", year%100, month, day, buildNumber);

	return str;
}

//*****************************************************************************
void VuStringUtil::replace(std::string &str, const char *strFind, const char *strReplace)
{
	size_t found = str.find(strFind);
	if ( found != std::string::npos )
		str.replace(found, strlen(strFind), strReplace);
}

//*****************************************************************************
void VuStringUtil::tokenize(const std::string &str, char delimeter, std::vector<std::string> &result)
{
	size_t pos0 = 0;
	size_t pos1 = str.find_first_of(delimeter);
	while ( pos1 != std::string::npos )
	{
		result.push_back(str.substr(pos0, pos1 - pos0));
		pos0 = pos1 + 1;
		pos1 = str.find_first_of(delimeter, pos0);
	}
	result.push_back(str.substr(pos0, pos1));
}

//*****************************************************************************
VUINT VuStringUtil::readInt(const char *str)
{
	VUINT value = 0;
	VU_SSCANF(str, "%d", &value);
	return value;
}

//*****************************************************************************
VUINT64 VuStringUtil::readInt64(const char *str)
{
	VUINT64 value = 0;
	VU_SSCANF(str, "%lld", &value);
	return value;
}

//*****************************************************************************
void VuStringUtil::toLower(std::string &str)
{
	for ( auto &c : str )
		c = tolower(c);
}

//*****************************************************************************
void VuStringUtil::toLower(char *str, VUUINT maxLen)
{
	while ( *str && maxLen )
	{
		*str = tolower(*str);
		str++;
		maxLen--;
	}
}