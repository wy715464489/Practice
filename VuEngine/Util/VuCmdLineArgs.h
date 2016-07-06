//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Command line arg utility
// 
//*****************************************************************************

#pragma once

class VuCmdLineArgs
{
public:
	char*		getNextArgument(const char* pString, int &delimiterSize);
	void		parse(const char *cmdLine);
	void		parse(int argc, const char *argv[]);
	bool		getValue(const char *key, std::string &value) const;
	bool		getValue(const char *key, bool &value) const;
	bool		getValue(const char *key, int &value) const;
	bool		getValue(const char *key, float &value) const;

private:
	typedef	std::map<std::string, std::string> Args;

	Args		mArgs;
};
