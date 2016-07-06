//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Registry
// 
//*****************************************************************************

#include "VuPfxRegistry.h"


//*****************************************************************************
void VuPfxRegistry::registerPattern(const char *type, const char *shortType, VuPfxPattern *(*createFn)())
{
	VUUINT32 hashedType = VuHash::fnv32String(type);

	// check for duplicate registration
	VUASSERT(mPatternTypes.find(hashedType) == mPatternTypes.end(), "VuPfxRegistry::registerPattern() duplicate registration");

	VuPatternTypeInfo entry;
	entry.mstrType = type;
	entry.mstrShortType = shortType;
	entry.mCreateFn = createFn;

	mPatternTypes[hashedType] = entry;
}

//*****************************************************************************
int VuPfxRegistry::getPatternTypeCount()
{
	return (int)mPatternTypes.size();
}

//*****************************************************************************
void VuPfxRegistry::getPatternTypeInfo(int index, std::string &type, std::string &shortType)
{
	PatternTypes::const_iterator iter = mPatternTypes.begin();
	for ( int i = 0; i < index; i++ )
		iter++;

	type = iter->second.mstrType;
	shortType = iter->second.mstrShortType;
}

//*****************************************************************************
const char *VuPfxRegistry::getPatternShortType(const char *type)
{
	VUUINT32 hashedType = VuHash::fnv32String(type);

	PatternTypes::const_iterator iter = mPatternTypes.find(hashedType);
	if ( iter != mPatternTypes.end() )
		return iter->second.mstrShortType;

	return "";
}

//*****************************************************************************
void VuPfxRegistry::registerProcess(const char *patternType, const char *type, const char *shortType, VuPfxProcess *(*createFn)())
{
	VuProcessTypeInfo entry;
	entry.mstrType = type;
	entry.mstrShortType = shortType;
	entry.mCreateFn = createFn;

	VUUINT32 hashedType = VuHash::fnv32String(type);

	if ( patternType[0] == '\0' )
	{
		mGenericProcessTypes[hashedType] = entry;
	}
	else
	{
		VUUINT32 hashedPatternType = VuHash::fnv32String(patternType);

		PatternTypes::iterator itPattern = mPatternTypes.find(hashedPatternType);
		if ( itPattern != mPatternTypes.end() )
		{
			VuPatternTypeInfo &patternInfo = itPattern->second;

			// check for duplicate registration
			VUASSERT(patternInfo.mProcessTypes.find(hashedType) == patternInfo.mProcessTypes.end(), "VuPfxRegistry::registerProcess() duplicate registration");

			patternInfo.mProcessTypes[hashedType] = entry;
		}
	}
}

//*****************************************************************************
int VuPfxRegistry::getProcessTypeCount(const char *patternType)
{
	int count = (int)mGenericProcessTypes.size();

	VUUINT32 hashedPatternType = VuHash::fnv32String(patternType);

	PatternTypes::iterator itPattern = mPatternTypes.find(hashedPatternType);
	if ( itPattern != mPatternTypes.end() )
		count += (int)itPattern->second.mProcessTypes.size();

	return count;
}

//*****************************************************************************
void VuPfxRegistry::getProcessTypeInfo(const char *patternType, int index, std::string &type, std::string &shortType)
{
	ProcessTypes::const_iterator iter;
	if ( index < (int)mGenericProcessTypes.size() )
	{
		iter = mGenericProcessTypes.begin();
		for ( int i = 0; i < index; i++ )
			iter++;
	}
	else
	{
		index -= (int)mGenericProcessTypes.size();

		VUUINT32 hashedPatternType = VuHash::fnv32String(patternType);

		PatternTypes::iterator itPattern = mPatternTypes.find(hashedPatternType);
		iter = itPattern->second.mProcessTypes.begin();
		for ( int i = 0; i < index; i++ )
			iter++;
	}

	type = iter->second.mstrType;
	shortType = iter->second.mstrShortType;
}

//*****************************************************************************
const char *VuPfxRegistry::getProcessShortType(const char *type)
{
	VUUINT32 hashedType = VuHash::fnv32String(type);

	ProcessTypes::const_iterator iter = mGenericProcessTypes.find(hashedType);
	if ( iter != mGenericProcessTypes.end() )
		return iter->second.mstrShortType;

	for ( PatternTypes::const_iterator itPattern = mPatternTypes.begin(); itPattern != mPatternTypes.end(); itPattern++ )
	{
		ProcessTypes::const_iterator iter = itPattern->second.mProcessTypes.find(hashedType);
		if ( iter != itPattern->second.mProcessTypes.end() )
			return iter->second.mstrShortType;
	}

	return "";
}

//*****************************************************************************
VuPfxPattern *VuPfxRegistry::createPattern(VUUINT32 hashedType)
{
	PatternTypes::const_iterator iter = mPatternTypes.find(hashedType);
	if ( iter != mPatternTypes.end() )
		return iter->second.mCreateFn();

	return VUNULL;
}

//*****************************************************************************
VuPfxProcess *VuPfxRegistry::createProcess(VUUINT32 hashedPatternType, VUUINT32 hashedType)
{
	ProcessTypes::const_iterator iter = mGenericProcessTypes.find(hashedType);
	if ( iter != mGenericProcessTypes.end() )
		return iter->second.mCreateFn();

	PatternTypes::iterator itPattern = mPatternTypes.find(hashedPatternType);
	if ( itPattern != mPatternTypes.end() )
	{
		ProcessTypes::const_iterator iter = itPattern->second.mProcessTypes.find(hashedType);
		if ( iter != itPattern->second.mProcessTypes.end() )
			return iter->second.mCreateFn();
	}

	return VUNULL;
}

