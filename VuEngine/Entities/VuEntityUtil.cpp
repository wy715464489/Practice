//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Entity name utility functionality.
// 
//*****************************************************************************

#include <ctype.h>
#include "VuEntityUtil.h"
#include "VuEngine/Entities/VuEntity.h"


//*****************************************************************************
std::string VuEntityUtil::getRoot(const std::string &strLongName)
{
	size_t slash = (int)strLongName.find_first_of('/');
	if ( slash != std::string::npos )
		return strLongName.substr(0, slash);
	return strLongName;
}

//*****************************************************************************
std::string VuEntityUtil::getRemainder(const std::string &strLongName)
{
	size_t slash = (int)strLongName.find_first_of('/');
	if ( slash != std::string::npos )
		return strLongName.substr(slash + 1);
	return "";
}

//*****************************************************************************
std::string VuEntityUtil::subtractRoot(const std::string &strLongName)
{
	size_t slash = (int)strLongName.find_first_of('/');
	if ( slash != std::string::npos )
		return strLongName.substr(slash + 1);
	return strLongName;
}

//*****************************************************************************
std::string VuEntityUtil::getPath(const std::string &strLongName)
{
	size_t slash = (int)strLongName.find_last_of('/');
	if ( slash != std::string::npos )
		return strLongName.substr(0, slash);
	return "";
}

//*****************************************************************************
std::string VuEntityUtil::getName(const std::string &strLongName)
{
	size_t slash = (int)strLongName.find_last_of('/');
	if ( slash != std::string::npos )
		return strLongName.substr(slash + 1);
	return strLongName;
}

//*****************************************************************************
std::string VuEntityUtil::generateName(const VuEntity *pParentEntity, std::string baseName)
{
	// strip any appended digits
	while ( baseName.size() && isdigit(baseName[baseName.size() - 1]) )
		baseName.resize(baseName.size() - 1);

	char name[256] = "";
	int i = 1;
	do
	{
		VU_SPRINTF(name, sizeof(name), "%s%02d", baseName.c_str(), i);
		i++;
	} while ( pParentEntity->getChildEntity(name) );

	return name;
}


//*****************************************************************************
int VuEntityUtil::getTotalPropertyCount(const VuEntity *pEntity)
{
	int count = pEntity->getProperties().getCount();
	for ( VuComponent *pComponent = pEntity->getComponentList().getFirst(); pComponent; pComponent = pComponent->getNextComponent() )
		count +=pComponent->getProperties().getCount();

	return count;
}

//*****************************************************************************
VuProperty *VuEntityUtil::getTotalProperty(const VuEntity *pEntity, int index)
{
	if ( index < pEntity->getProperties().getCount() )
		return pEntity->getProperties().getByIndex(index);

	index -= pEntity->getProperties().getCount();

	for ( VuComponent *pComponent = pEntity->getComponentList().getFirst(); pComponent; pComponent = pComponent->getNextComponent() )
	{
		if ( index < pComponent->getProperties().getCount() )
			return pComponent->getProperties().getByIndex(index);

		index -= pComponent->getProperties().getCount();
	}

	return VUNULL;
}