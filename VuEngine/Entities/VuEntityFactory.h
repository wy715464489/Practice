//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Factory class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEntity;
class VuDBAsset;

class VuEntityFactory : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuEntityFactory)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();

public:
	// register entities
	void					registerEntity(const std::string &strType, VuEntity *(*createFn)(const char *strType));
	void					finalizeRegistration();
	
	// creation
	VuEntity				*createEntity(const std::string &strType);
	template <class T> T	*createEntity() { return static_cast<T *>(createEntity(T::msRTTI.mstrType)); }

	// enumeration
	int						getNumTypes() { return (int)mOrder.size(); }
	const std::string		&getType(int index) { return mTypes[mOrder[index]].mType; }
	const std::string		&getPath(int index) { return mTypes[mOrder[index]].mPath; }
	const std::string		&getShortType(int index) { return mTypes[mOrder[index]].mShortType; }

	// info
	const std::string		&getShortType(const std::string &strType);

private:
	void					registerTemplates();

	typedef VuEntity *(*CreateFn)(const char *strType);
	struct VuTypeInfo
	{
		std::string	mType;
		std::string	mPath;
		std::string	mShortType;
		CreateFn	mCreateFn;
	};
	typedef std::vector<VuTypeInfo> Types;
	typedef std::hash_map<VUUINT32, int> Lookup;
	typedef std::vector<int> Order;
	Types		mTypes;
	Lookup		mLookup;
	Order		mOrder;
};


//*****************************************************************************
// Macro used by registries to register entities from their init() method.
#define REGISTER_ENTITY(type) \
{ \
	extern VuEntity *Create##type(const char *strType); \
	VuEntityFactory::IF()->registerEntity(#type, Create##type); \
}

