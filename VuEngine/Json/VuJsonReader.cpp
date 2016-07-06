//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to read data containers.
// 
//*****************************************************************************

#include <stdarg.h>
#include "VuJsonReader.h"
#include "VuJsonContainer.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBase64.h"


//*****************************************************************************
VuJsonReader::VuJsonReader():
	mpCurTok(0)
{
}

//*****************************************************************************
bool VuJsonReader::loadFromFile(VuJsonContainer &container, const std::string &strFileName)
{
	mstrError.clear();

	// open file
	VUHANDLE fp = VuFile::IF()->open(strFileName, VuFile::MODE_READ);
	if ( fp == VUNULL )
		return error("Unable to open for reading: %s", strFileName.c_str());

	// get file size
	int fileSize =  VuFile::IF()->size(fp);

	// read file
	char *strFile = new char[fileSize+1];
	VuFile::IF()->read(fp, strFile, fileSize);
	strFile[fileSize] = '\0';

	// check for and skip UTF-8 header
	const char *strDoc = strFile;
	if ( (VUUINT8)strDoc[0] == 0xef && (VUUINT8)strDoc[1] == 0xbb && (VUUINT8)strDoc[2] == 0xbf )
		strDoc += 3;

	// read file
	bool bSuccess = loadFromString(container, strDoc);

	// clean up
	delete[] strFile;
	VuFile::IF()->close(fp);

	// load data
	return bSuccess;
}

//*****************************************************************************
bool VuJsonReader::loadFromString(VuJsonContainer &container, const std::string &str)
{
	return loadFromString(container, str.c_str());
}

//*****************************************************************************
bool VuJsonReader::loadFromString(VuJsonContainer &container, const char *str)
{
	mstrError.clear();
	mpCurTok = str;

	// start w/ empty container
	container.clear();

	// recursively read containers
	if ( !readContainer(container) )
	{
		container.clear();
		return false;
	}

	// make sure that there's no garbage following data
	if ( nextToken() )
	{
		// unexpected trailing token
		container.clear();
		return error("Expecting end of document: %s", mpCurTok);
	}

	return true;
}

//*****************************************************************************
const std::string &VuJsonReader::getLastError()
{
	return mstrError;
}

//*****************************************************************************
bool VuJsonReader::error(const char *fmt, ...)
{
	va_list args;
	char str[256];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	mstrError += str;
	mstrError += '\n';

	return false;
}

//*****************************************************************************
char VuJsonReader::nextToken()
{
	while ( char c = *mpCurTok )
	{
		if ( c == '/' )
		{
			skipComment();
		}
		else if ( c != ' ' && c != '\t' && c != '\r' && c != '\n' )
		{
			return c;
		}
		else
		{
			mpCurTok++; // skip whitespace
		}
	}

	return 0; // eof
}

//*****************************************************************************
bool VuJsonReader::skipComment()
{
	mpCurTok++; // skip /
	if ( *mpCurTok == '*' )
		return skipCStyleComment();
	else if ( *mpCurTok == '/' )
		return skipCppStyleComment();
	return error("Invalid comment: %s", mpCurTok);
}

//*****************************************************************************
bool VuJsonReader::skipCStyleComment()
{
	if ( const char *end = strstr(mpCurTok, "*/") )
	{
		mpCurTok = end + 2; // skip */
		return true;
	}

	return error("End of C-style comment not found: %s", mpCurTok);
}

//*****************************************************************************
bool VuJsonReader::skipCppStyleComment()
{
	if ( const char *end = strpbrk(mpCurTok, "\r\n") )
	{
		mpCurTok = end + 1; // skip eol
		return true;
	}

	// end of file
	mpCurTok += strlen(mpCurTok);

	return true;
}

//*****************************************************************************
bool VuJsonReader::readContainer(VuJsonContainer &container)
{
	char tok = nextToken();

	switch ( tok )
	{
		case '{':
		{
			return readObject(container);
			break;
		}
		case '[':
		{
			return readArray(container);
			break;
		}
		case '"':
		{
			return readString(container);
			break;
		}
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
		{
			return readNumber(container);
			break;
		}
		case 't':
		case 'f':
		{
			return readBool(container);
			break;
		}
		case 'n':
		{
			return readNull(container);
		}
		case '\0' :
		{
			return true;
		}
	};

	return error("Invalid token: %s", mpCurTok);
}

//*****************************************************************************
bool VuJsonReader::readObject(VuJsonContainer &container)
{
	mpCurTok++; // skip {

	// check for empty object
	char tok = nextToken();
	if ( tok == '}' )
	{
		mpCurTok++; // skip }
		container.makeObject();
		return true;
	}

	bool bDone = false;
	while ( !bDone )
	{
		// read key
		tok = nextToken();
		if ( tok != '"' )
			return error("Object parsing error, expecting '\"': %s", mpCurTok);

		std::string strKey;
		if ( !readString(strKey) )
			return false;

		// check for duplicate
		if ( container.hasMember(strKey) )
			return error("Duplicate object member: %s", strKey.c_str());

		// find :
		tok = nextToken();
		if ( tok != ':' )
			return error("Object parsing error, expecting ':': %s", mpCurTok);
		mpCurTok++; // skip :

		// recurse
		if ( !readContainer(container[strKey]) )
			return false;

		// look for } or ,
		tok = nextToken();
		if ( tok == '}' )
		{
			mpCurTok++; // skip }
			bDone = true;
		}
		else if ( tok == ',' )
		{
			mpCurTok++; // skip ,

			// allow for trailing commas
			if ( nextToken() == '}' )
			{
				mpCurTok++; // skip }
				bDone = true;
			}
		}
		else
		{
			return error("Object parsing error, expecting '}' or ',': %s", mpCurTok);
		}
	}

	// detect binary data
	if ( container.numMembers() == 1 )
	{
		const std::string &key = container.getMemberKey(0);
		if ( key == "BinaryBase64" )
		{
			const VuJsonContainer &value = container[key];
			if ( value.isString() )
			{
				VuArray<VUBYTE> bytes;
				if ( !VuBase64::decode(value.asCString(), bytes) )
					return error("Binary data parsing error: %s", mpCurTok);

				container.putValue(&bytes[0], bytes.size());
			}
		}
	}

	return true;
}

//*****************************************************************************
bool VuJsonReader::readArray(VuJsonContainer &container)
{
	mpCurTok++; // skip [

	// check for empty array
	char tok = nextToken();
	if ( tok == ']' )
	{
		mpCurTok++; // skip ]
		container.makeArray();
		return true;
	}

	while ( readContainer(container.append()) )
	{
		// look for ] or ,
		tok = nextToken();
		if ( tok == ']' )
		{
			mpCurTok++; // skip ]
			return true;
		}
		else if ( tok == ',' )
		{
			mpCurTok++; // skip ,

			// allow for trailing commas
			if ( nextToken() == ']' )
			{
				mpCurTok++; // skip ]
				return true;
			}
		}
		else
		{
			return error("Array parsing error, expecting ']' or ',': %s", mpCurTok);
		}
	}

	return false;
}

//*****************************************************************************
bool VuJsonReader::readString(VuJsonContainer &container)
{
	std::string str;
	if ( readString(str) )
	{
		container.putValue(str);
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuJsonReader::readNumber(VuJsonContainer &container)
{
	// hex?
	bool bHex = (mpCurTok[0] == '0' && mpCurTok[1] == 'x');
	
	// get string size of number
	int size = 1;
	while ( char c = mpCurTok[size] )
	{
		if ( !(c >= '0' && c <= '9') && 
			c != '.' && 
			c != 'e' && 
			c != 'E' && 
			c != '+' && 
			c != '-' &&
			c != 'x' &&
			!((c >= 'a' && c <= 'f') ||
			  (c >= 'A' && c <= 'F')))
			break;
		size++;
	}

	// check for float
	if(!bHex)
	{
		for ( int i = 1; i < size; i++ )
		{
			if ( !(mpCurTok[i] >= '0' && mpCurTok[i] <= '9'))
			{
				return readFloat(size, container);
			}
		}
	}

	// parse as integer
	int base;
	
	if(bHex)
	{
		base = 16;
	}
	else
	{
		base = 10;
	}

	// negative?
	bool bNeg = (mpCurTok[0] == '-');

	// parse digits
	VUINT64 val = 0;
	int start = 0;

	if(bHex)
	{
		// skip the '0' and 'x'
		start = 2;
	}
	else if(bNeg)
	{
		// skip the '-'
		start = 1;
	}

	for(int i = start; i < size; i++)
	{
		char c = mpCurTok[i];

		VUINT64 digit;

		if(c >= 'a' && c <= 'f')
		{
			digit = 10 + VUINT64(c - 'a');
		}
		else if(c >= 'A' && c <= 'F')
		{
			digit = 10 + VUINT64(c - 'A');
		}
		else
		{
			digit = VUINT64(c - '0');
		}

		val = val * base + digit;
	}

	// store as 32 or 64 bit int as appropriate
	VUINT64 i64Val = bNeg ? -val : val;
	if ( i64Val < INT_MIN || i64Val > INT_MAX )
	{
		container.putValue(i64Val);
	}
	else
	{
		int iVal = int(i64Val);
		container.putValue(iVal);
	}

	mpCurTok += size; // skip number

	return true;
}

//*****************************************************************************
bool VuJsonReader::readBool(VuJsonContainer &container)
{
	if ( strncmp(mpCurTok, "true", 4) == 0 )
	{
		container.putValue(true);
		mpCurTok += 4; // skip true
		return true;
	}
	else if ( strncmp(mpCurTok, "false", 5) == 0 )
	{
		container.putValue(false);
		mpCurTok += 5; // skip false
		return true;
	}

	return error("Invalid token: %s", mpCurTok);
}

//*****************************************************************************
bool VuJsonReader::readNull(VuJsonContainer &container)
{
	if ( strncmp(mpCurTok, "null", 4) == 0 )
	{
		container = VuJsonContainer::null;
		mpCurTok += 4; // skip null
		return true;
	}

	return error("Invalid token: %s", mpCurTok);
}

//*****************************************************************************
bool VuJsonReader::readString(std::string &strVal)
{
	mpCurTok++; // skip leading "

	// find trailing "
	const char *end = mpCurTok;
	while ( (end = strchr(end, '"')) != 0)
	{
		if ( end[-1] != '\\' || end[-2] == '\\' )
			break;
		end++;
	}
	if ( !end )
		return error("String parsing error, trailing \" not found: %s", mpCurTok);

	// decode
	strVal.reserve(end - mpCurTok);
	while ( mpCurTok != end )
	{
		char c = *mpCurTok;
		if ( c == '\\' )
		{
			mpCurTok++; // skip '\'
			switch ( *mpCurTok )
			{
				case '"': strVal += '"'; break;
				case '\\': strVal += '\\'; break;
				case '/': strVal += '/'; break;
				case 'b': strVal += '\b'; break;
				case 'f': strVal += '\f'; break;
				case 'n': strVal += '\n'; break;
				case 'r': strVal += '\r'; break;
				case 't': strVal += '\t'; break;
				case 'u':
				{
					mpCurTok++; // skip u

					if ( end - mpCurTok < 4 )
						return error("String parsing error, invalid unicode escape sequence: %s", mpCurTok);

					// decode unicode escape sequence
					int unicode = decodeUnicodeSequence(mpCurTok);
					if ( unicode == 0 )
						return error("String parsing error, invalid unicode escape sequence: %s", mpCurTok);

					// encode in utf-8
					VuUtf8::appendUnicodeToUtf8String(unicode, strVal);

					// skip sequence
					mpCurTok += 3;

					break;
				}
				default:
				{
					return error("String parsing error, invalid escape sequence: %s", mpCurTok);
					break;
				}
			}
		}
		else
		{
			strVal += c;
		}

		mpCurTok++;
	}

	mpCurTok = end + 1; // skip trailing "

	return true;
}

//*****************************************************************************
bool VuJsonReader::readFloat(int size, VuJsonContainer &container)
{
	char buffer[256];
	if ( size >= sizeof(buffer) )
		return error("Float parsing error: %s", mpCurTok);

	VU_MEMCPY(buffer, sizeof(buffer), mpCurTok, size);
	buffer[size] = '\0';

	float fVal;
	if ( VU_SSCANF(buffer, "%g", &fVal) != 1 )
		return error("Float parsing error: %s", mpCurTok);

	container.putValue(fVal);

	mpCurTok += size; // skip number

	return true;
}

//*****************************************************************************
int VuJsonReader::decodeUnicodeSequence(const char *str)
{
	int unicode = 0;
	for ( int i = 0; i < 4; i++ )
	{
		char c = str[i];

		if ( c >= '0' && c <= '9' )			c = c - '0';
		else if ( c >= 'a' && c <= 'f' )	c = c - 'a' + 10;
		else if ( c >= 'A' && c <= 'F' )	c = c - 'A' + 10;
		else
			return 0;

		unicode = (unicode << 4) | c;
	}

	return unicode;
}
