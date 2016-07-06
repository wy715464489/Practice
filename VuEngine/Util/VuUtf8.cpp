//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Utility functionality to deal with utf-8.
// 
//*****************************************************************************

#include "VuUtf8.h"

//*****************************************************************************
int VuUtf8::convertUtf8ToUnicode(const char *utf8, VUUINT32 &unicode)
{
	// single byte (most common case)
	if ( (utf8[0] & 0x80) == 0 )
	{
		unicode = *utf8;
		return 1;
	}

	// 2 bytes
	if ( (utf8[0] & 0xE0) == 0xC0 )
	{
		// check for encoding errors
		if ( (utf8[1] & 0xC0 ) != 0x80 )	return 0;

		unsigned int byte1 = (utf8[0] & 0x1F);
		unsigned int byte2 = (utf8[1] & 0x3F);

		unicode = (byte1 << 6) | byte2;

		return 2;
	}

	// 3 bytes
	if ( (utf8[0] & 0xF0) == 0xE0 )
	{
		// check for encoding errors
		if ( (utf8[1] & 0xC0 ) != 0x80 )	return 0;
		if ( (utf8[2] & 0xC0 ) != 0x80 )	return 0;

		unsigned int byte1 = (utf8[0] & 0x0F);
		unsigned int byte2 = (utf8[1] & 0x3F);
		unsigned int byte3 = (utf8[2] & 0x3F);

		unicode = (byte1 << 12) | (byte2 << 6) | byte3;

		return 3;
	}

	// 4 bytes
	if ( (utf8[0] & 0xF8) == 0xF0 )
	{
		// check for encoding errors
		if ( (utf8[1] & 0xC0 ) != 0x80 )	return 0;
		if ( (utf8[2] & 0xC0 ) != 0x80 )	return 0;
		if ( (utf8[3] & 0xC0 ) != 0x80 )	return 0;

		unsigned int byte1 = (utf8[0] & 0x07);
		unsigned int byte2 = (utf8[1] & 0x3F);
		unsigned int byte3 = (utf8[2] & 0x3F);
		unsigned int byte4 = (utf8[3] & 0x3F);

		unicode = (byte1 << 18) | (byte2 << 12) | (byte3 << 6) | byte4;

		// handle RFC 3629
		//   UTF-8 was restricted by RFC 3629 to use only the area covered by the formal Unicode
		//   definition, U+0000 to U+10FFFF, in November 2003.
		if ( unicode > 0x10FFFF )
			return 0;

		return 4;
	}

	return 0;
}

//*****************************************************************************
int VuUtf8::convertUnicodeToUtf8(VUUINT32 unicode, char *utf8)
{
	// single byte (most common case)
	if ( unicode <= 0x7F )
	{
		if ( utf8 )
		{
			utf8[0] = unicode & 0x7F;
		}

		return 1;
	}

	// 2 bytes
	if ( unicode <= 0x7FF )
	{
		if ( utf8 )
		{
			utf8[0] = (char)(0xC0 | (unicode >> 6));
			utf8[1] = (char)(0x80 | (unicode & 0x3F));
		}

		return 2;
	}

	// 3 bytes
	if ( unicode <= 0xFFFF )
	{
		if ( utf8 )
		{
			utf8[0] = (char)(0xE0 | (unicode >> 12));
			utf8[1] = (char)(0x80 | ((unicode >> 6) & 0x3F));
			utf8[2] = (char)(0x80 | (unicode & 0x3F));
		}

		return 3;
	}

	// 4 bytes
	if ( unicode <= 0x10FFFF ) // limited by RFC 3629
	{
		if ( utf8 )
		{
			utf8[0] = (char)(0xF0 | (unicode >> 18));
			utf8[1] = (char)(0x80 | ((unicode >> 12) & 0x3F));
			utf8[2] = (char)(0x80 | ((unicode >> 6) & 0x3F));
			utf8[3] = (char)(0x80 | (unicode & 0x3F));
		}

		return 4;
	}

	return 0;
}

//*****************************************************************************
VUUINT VuUtf8::convertUtf8StringToWCharString(const char *pUtf8, wchar_t *pWChar, VUUINT length)
{
	size_t iWChar = 0;
	size_t iUtf8 = 0;
	
	while(iWChar < length && pUtf8[iUtf8] != 0)
	{
		VUUINT32 unicode;

		int bytes = VuUtf8::convertUtf8ToUnicode(&pUtf8[iUtf8], unicode);

		if(!bytes)
		{
			pWChar[iWChar] = 0;
			return (VUUINT)iWChar;
		}

		iUtf8 += bytes;

		// NOTE: truncates from VUUINT32 to wchar_t
		pWChar[iWChar++] = (wchar_t)unicode;
	}

	if ( iWChar < length )
		pWChar[iWChar] = 0;
	else
		pWChar[length-1] = 0;

	return (VUUINT)iWChar;
}

//*****************************************************************************
void VuUtf8::convertUtf8StringToWCharString(const char *pUtf8, std::wstring &unicodeString)
{
	unicodeString.clear();

	while ( *pUtf8 )
	{
		VUUINT32 unicode;
		int bytes = VuUtf8::convertUtf8ToUnicode(pUtf8, unicode);
		if(!bytes)
			return;

		unicodeString.push_back((wchar_t)unicode);

		pUtf8 += bytes;
	}
}

//*****************************************************************************
void VuUtf8::convertWCharStringToUtf8String(const wchar_t *unicodeString, std::string &utf8String)
{
	utf8String.clear();

	while ( *unicodeString )
	{
		appendUnicodeToUtf8String(*unicodeString, utf8String);
		unicodeString++;
	}
}

//*****************************************************************************
int VuUtf8::appendUnicodeToUtf8String(VUUINT32 unicode, std::string &utf8)
{
	char str[5];

	int bytes = convertUnicodeToUtf8(unicode, str);
	str[bytes] = '\0';

	utf8 += str;

	return bytes;
}

//*****************************************************************************
int VuUtf8::appendUnicodeStringToUtf8String(const VUUINT32 *unicodeString, std::string &utf8)
{
	int bytes = 0;
	while ( *unicodeString )
	{
		bytes += appendUnicodeToUtf8String(*unicodeString, utf8);
		unicodeString++;
	}
	return bytes;
}

//*****************************************************************************
int VuUtf8::appendUnicodeStringToUtf8String(const wchar_t *unicodeString, std::string &utf8)
{
	int bytes = 0;
	while ( *unicodeString )
	{
		bytes += appendUnicodeToUtf8String(*unicodeString, utf8);
		unicodeString++;
	}
	return bytes;
}

//*****************************************************************************
int VuUtf8::appendAsciiStringToUtf8String(const char *asciiString, std::string &utf8)
{
	int bytes = 0;
	while ( *asciiString )
	{
		bytes += appendUnicodeToUtf8String((VUUINT8)*asciiString, utf8);
		asciiString++;
	}
	return bytes;
}