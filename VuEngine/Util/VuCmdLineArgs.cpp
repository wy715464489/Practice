//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Command line arg utility
// 
//*****************************************************************************

#include "VuCmdLineArgs.h"

// Added the ability to specify argument delimiters other than --
const char* sDelimiters[] = 
{
	"--",
	"+",
	NULL
};

//*****************************************************************************
char* VuCmdLineArgs::getNextArgument(const char* pString, int &delimiterSize)
{
	int whichDelimiter = 0;
	const char *pNext = NULL;

	// See which delimiter is "closest" to where we are
	const char* pEnd = pString + strlen(pString);
	char* pClosest = (char *)pEnd;

	while (sDelimiters[whichDelimiter] != NULL)
	{
		pNext = strstr(pString, sDelimiters[whichDelimiter]);
		if (pNext && pNext < pClosest)
		{
			// Skip the delimiters
			pClosest = (char *)pNext;
			delimiterSize = (int)strlen(sDelimiters[whichDelimiter]);
		}
		whichDelimiter++;
	}

	if (pClosest == pEnd)
	{
		return NULL;
	}
	else
	{
		return pClosest;
	}
}

//*****************************************************************************
void VuCmdLineArgs::parse(const char *cmdLine)
{
	char str[256];
	VU_STRCPY(str, sizeof(str), cmdLine);

	int delimiterSize;

	// Modified the parse function to use getNextArgument() instead of strstr() with "--" as 
	// the delimiter so that multiple delimiters can be used in the same command line
	//
	char *next = getNextArgument(str, delimiterSize);
	while ( next )
	{
		char *cur = next + delimiterSize;
		next = getNextArgument(cur, delimiterSize);
		if ( next )
			*next = '\0';

		// kill trailing spaces
		while ( strlen(cur) && cur[strlen(cur)-1] == ' ' )
			cur[strlen(cur)-1] = '\0';

		if ( char *p = strchr(cur, ' ') )
		{
			*p = '\0';
			p++;
			
			// remove surrounding "s if present
			if ( p[0] == '\"' && p[strlen(p)-1] == '\"' )
			{
				p[strlen(p)-1] = '\0';
				p++;
			}
			
			mArgs[cur] = p;
		}
		else
		{
			mArgs[cur] = "";
		}
	}
}

//*****************************************************************************
void VuCmdLineArgs::parse(int argc, const char *argv[])
{
	char cmdLine[256] = "";

	for ( int i = 0; i < argc; i++ )
	{
		VU_STRCAT(cmdLine, sizeof(cmdLine), argv[i]);
		VU_STRCAT(cmdLine, sizeof(cmdLine), " ");
	}

	parse(cmdLine);
}

//*****************************************************************************
bool VuCmdLineArgs::getValue(const char *key, std::string &value) const
{
	Args::const_iterator iter = mArgs.find(key);
	if ( iter == mArgs.end() )
		return false;

	value = iter->second;

	return true;
}

//*****************************************************************************
bool VuCmdLineArgs::getValue(const char *key, bool &value) const
{
	Args::const_iterator iter = mArgs.find(key);
	if ( iter == mArgs.end() )
		return false;

	if ( iter->second == "true" )
		value = true;
	else if ( iter->second == "false" )
		value = false;
	else
		return false;

	return true;
}

//*****************************************************************************
bool VuCmdLineArgs::getValue(const char *key, int &value) const
{
	Args::const_iterator iter = mArgs.find(key);
	if ( iter == mArgs.end() )
		return false;

	return VU_SSCANF(iter->second.c_str(), "%d", &value) == 1;
}

//*****************************************************************************
bool VuCmdLineArgs::getValue(const char *key, float &value) const
{
	Args::const_iterator iter = mArgs.find(key);
	if ( iter == mArgs.end() )
		return false;

	return VU_SSCANF(iter->second.c_str(), "%g", &value) == 1;
}
