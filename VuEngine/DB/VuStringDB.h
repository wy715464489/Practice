//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String DB class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuHashedString;

class VuStringDB : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuStringDB)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init() = 0;

public:
	virtual const std::string	&getString(const std::string &id) const = 0;
	virtual const std::string	&getString(const char *id) const = 0;
	virtual const std::string	&getStringSelf(const std::string &id) const = 0;
	virtual const char			*getStringSelf(const char *id) const = 0;

	virtual const std::string	&getMissingString() const = 0;

	virtual const std::string	&getCurrentLanguageCode() const = 0;
	virtual bool				setCurrentLanguageCode(const std::string &languageCode) = 0;
	virtual bool				isCurrentLanguageEastAsian() const = 0;

	virtual int					getNumLanguages() const = 0;
	virtual const std::string	&getLanguageCode(int index) const = 0;
	virtual bool				doesLanguageExist(const std::string &language) const = 0;

	virtual bool				reload() = 0;

	virtual bool				exportToFile(const std::string &absFileName) = 0;
	virtual bool				importFromFile(const std::string &absFileName) = 0;

	static void					dumpCharacterMap(const std::string &languageCodes, std::wstring &characters);
};
