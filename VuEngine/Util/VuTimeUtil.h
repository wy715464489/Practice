//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Time utility
// 
//*****************************************************************************

#pragma once


namespace VuTimeUtil
{
	struct VuTimeStruct
	{
		int mYear;
		int mMonth;  // [1,12]
		int mDay;    // [1,31]
		int mHour;   // [0,23]
		int mMinute; // [0,59]
		int mSecond; // [0,59]
	};

	void	getLocalTime(VuTimeStruct &timeStruct);
	void	getCompileTime(VuTimeStruct &timeStruct);

	VUINT64	diffTime(const VuTimeStruct &tsa, const VuTimeStruct &tsb); // tsa - tsb in seconds

	int		calcDaysInMonth(int year, int month);
	int		calcDaysSince2000(int year, int month, int day);
	int		calcDaysSince2000();
	VUINT64	calcSecondsSince2000();
}
