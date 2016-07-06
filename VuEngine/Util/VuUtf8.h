//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Utility functionality to deal with utf-8.
// 
//*****************************************************************************

#pragma once

namespace VuUtf8
{
	// returns number of bytes converted (0 if error)
	int convertUtf8ToUnicode(const char *utf8, VUUINT32 &unicode);

	// returns number of bytes written (0 if error)
	// pass in VUNULL for *utf8 to get required size
	int convertUnicodeToUtf8(VUUINT32 unicode, char *utf8);

	// Converts a whole string of utf8 characters to wchar_t characters, up to length in size
	VUUINT convertUtf8StringToWCharString(const char *pUtf8, wchar_t *pUnicode, VUUINT length);

	void convertUtf8StringToWCharString(const char *pUtf8, std::wstring &unicodeString);
	void convertWCharStringToUtf8String(const wchar_t *unicodeString, std::string &utf8String);

	// helper functions, using stl string
	int appendUnicodeToUtf8String(VUUINT32 unicode, std::string &utf8);
	int appendUnicodeStringToUtf8String(const VUUINT32 *unicodeString, std::string &utf8);
	int appendUnicodeStringToUtf8String(const wchar_t *unicodeString, std::string &utf8);
	int appendAsciiStringToUtf8String(const char *asciiString, std::string &utf8);
}