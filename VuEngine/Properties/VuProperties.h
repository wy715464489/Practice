//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Properties class.
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"

class VuFastContainer;
class VuJsonContainer;

class VuProperties
{
public:
	VuProperties() : mpHead(VUNULL) {}
	~VuProperties();

	// load/save/reset
	void		load(const VuFastContainer &data);
	void		load(const VuJsonContainer &data);
	void		save(VuJsonContainer &data) const;
	void		updateDefaults();
	void		reset();

	// add/remove/clear
	VuProperty	*add(VuProperty *pProperty);
	void		remove(VuProperty *pProperty);
	void		clear();

	// access
	inline bool			hasProperties() const { return mpHead != VUNULL; }
	inline VuProperty	*get(const std::string &strName) const;
	inline VuProperty	*get(VUUINT32 hashedName) const;
	inline VuProperty	*getFirst() const { return mpHead; }
	inline int			getCount() const;
	inline VuProperty	*getByIndex(int index) const;

private:
	VuProperty	*mpHead;
};


//*****************************************************************************
inline VuProperty *VuProperties::get(const std::string &strName) const
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		if ( strcmp(pProperty->getName(), strName.c_str()) == 0 )
			return pProperty;

	return VUNULL;
}

//*****************************************************************************
inline VuProperty *VuProperties::get(VUUINT32 hashedName) const
{
	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		if ( pProperty->mHashedName == hashedName )
			return pProperty;

	return VUNULL;
}

//*****************************************************************************
inline int VuProperties::getCount() const
{
	int count = 0;

	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
		count++;

	return count;
}

//*****************************************************************************
inline VuProperty *VuProperties::getByIndex(int index) const
{
	int count = 0;

	for ( VuProperty *pProperty = mpHead; pProperty; pProperty = pProperty->mpNextProperty )
	{
		if ( count == index )
			return pProperty;
		count++;
	}

	return VUNULL;
}
