//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Registry
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Util/VuHash.h"

class VuPfxPattern;
class VuPfxProcess;
class VuPfxPatternInstance;
class VuPfxProcessInstance;


class VuPfxRegistry
{
public:
	// pattern registration
	void					registerPattern(const char *type, const char *shortType, VuPfxPattern *(*createFn)());
	int						getPatternTypeCount();
	void					getPatternTypeInfo(int index, std::string &type, std::string &shortType);
	const char				*getPatternShortType(const char *type);

	// process registration
	void					registerProcess(const char *patternType, const char *type, const char *shortType, VuPfxProcess *(*createFn)());
	int						getProcessTypeCount(const char *patternType);
	void					getProcessTypeInfo(const char *patternType, int index, std::string &type, std::string &shortType);
	const char				*getProcessShortType(const char *type);

	// creation
	VuPfxPattern			*createPattern(const char *type) { return createPattern(VuHash::fnv32String(type)); }
	VuPfxProcess			*createProcess(const char *patternType, const char *type) { return createProcess(VuHash::fnv32String(patternType), VuHash::fnv32String(type)); }
	VuPfxPattern			*createPattern(VUUINT32 hashedType);
	VuPfxProcess			*createProcess(VUUINT32 hashedPatternType, VUUINT32 hashedType);

private:
	struct VuProcessTypeInfo
	{
		const char				*mstrType;
		const char				*mstrShortType;
		VuPfxProcess			*(*mCreateFn)();
	};
	typedef std::map<VUUINT32, VuProcessTypeInfo> ProcessTypes;

	struct VuPatternTypeInfo
	{
		const char				*mstrType;
		const char				*mstrShortType;
		VuPfxPattern			*(*mCreateFn)();
		ProcessTypes			mProcessTypes;
	};
	typedef std::map<VUUINT32, VuPatternTypeInfo> PatternTypes;

	PatternTypes		mPatternTypes;
	ProcessTypes		mGenericProcessTypes;
};


//*****************************************************************************
// Macro used by to register patterns.
#define REGISTER_PFX_PATTERN(type, shortType)									\
{																				\
	extern VuPfxPattern *Create##type();										\
	VuPfx::IF()->registry()->registerPattern(#type, shortType, Create##type);	\
}

//*****************************************************************************
// Macro used by to register processes.
#define REGISTER_PFX_PROCESS(type, shortType, patternType)									\
{																							\
	extern VuPfxProcess *Create##type();													\
	VuPfx::IF()->registry()->registerProcess(patternType, #type, shortType, Create##type);	\
}
