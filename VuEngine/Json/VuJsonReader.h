//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Class used to read data containers.
// 
//*****************************************************************************

#pragma once


class VuJsonContainer;


class VuJsonReader
{
public:
	VuJsonReader();

	bool				loadFromFile(VuJsonContainer &container, const std::string &strFileName);
	bool				loadFromString(VuJsonContainer &container, const std::string &str);
	bool				loadFromString(VuJsonContainer &container, const char *str);

	const std::string	&getLastError();
	
private:

	bool			error(const char *fmt, ...);

	char			nextToken();
	bool			skipComment();
	bool			skipCStyleComment();
	bool			skipCppStyleComment();

	bool			readContainer(VuJsonContainer &container);
	bool			readObject(VuJsonContainer &container);
	bool			readArray(VuJsonContainer &container);
	bool			readString(VuJsonContainer &container);
	bool			readNumber(VuJsonContainer &container);
	bool			readBool(VuJsonContainer &container);
	bool			readNull(VuJsonContainer &container);

	bool			readString(std::string &strVal);
	bool			readFloat(int size, VuJsonContainer &container);

	int				decodeUnicodeSequence(const char *str);

	const char		*mpCurTok;
	std::string		mstrError;
};