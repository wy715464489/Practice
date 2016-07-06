//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Time format utility.
// 
//*****************************************************************************

#pragma once

namespace VuStringUtil
{
	enum eTimeFormat
	{
		HH_MM,
		HH_MM_SS,
		MM_SS,
		MM_SS_HH,
		MM_SS_TTT,
		SS,
		SS_HH,
		SS_TTT,
	};

	const char *timeFormatMilliseconds(eTimeFormat format, VUINT ms, char *str, VUUINT maxLen);
	const char *timeFormatSeconds(eTimeFormat format, float seconds, char *str, VUUINT maxLen);
	const char *timeFormatSeconds(eTimeFormat format, int seconds, char *str, VUUINT maxLen);

	enum eDateFormat
	{
		DD_MM_YYYY,
		MM_DD_YYYY,
		YYYY_MM_DD,
	};
	const char *dateFormat(eDateFormat format, int year, int month, int day, char *str, VUUINT maxLen);

	enum eCurrencyFormat
	{
		USD_CC,
		USD,
	};
	const char *currencyFormat(eCurrencyFormat format, float dollars, char *str, VUUINT maxLen);
	const char *currencyFormat(eCurrencyFormat format, int dollars, int cents, char *str, VUUINT maxLen);

	enum eFloatFormat
	{
		D_T,
		D_HH,
		D_TTT,
	};
	const char *floatFormat(eFloatFormat format, float value, char *str, VUUINT maxLen);
	const char *integerFormat(int value, char *str, VUUINT maxLen);

	const char *buildNumberFormat(int buildNumber, char *str, VUUINT maxLen);

	// replace strFind with strReplace in str
	void replace(std::string &str, const char *strFind, const char *strReplace);

	// tokenize string (this is slow, meant for tools)
	void tokenize(const std::string &str, char delimeter, std::vector<std::string> &result);

	VUINT readInt(const char *str);
	VUINT64 readInt64(const char *str);

	void toLower(char *str, VUUINT maxLen);
	void toLower(std::string &str);
}

