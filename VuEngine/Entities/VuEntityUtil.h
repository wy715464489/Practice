//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Entity name utility functionality.
// 
//*****************************************************************************

#pragma once

class VuEntity;
class VuProperty;


namespace VuEntityUtil
{
	std::string	getRoot(const std::string &strLongName);
	std::string	getRemainder(const std::string &strLongName);
	std::string	subtractRoot(const std::string &strLongName);
	std::string	getPath(const std::string &strLongName);
	std::string	getName(const std::string &strLongName);

	std::string	generateName(const VuEntity *pParentEntity, std::string baseName);

	int			getTotalPropertyCount(const VuEntity *pEntity);
	VuProperty	*getTotalProperty(const VuEntity *pEntity, int index);
}
