//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Entity class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Properties/VuProperties.h"
#include "VuEngine/Components/VuComponentList.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Events/VuEventMap.h"
#include "VuEngine/Util/VuHash.h"

class VuComponent;
class VuJsonContainer;
class VuTemplateAsset;
class VuTransformComponent;


class VuEntity : public VuRefObj
{
	DECLARE_RTTI
	DECLARE_EVENT_MAP

public:
	VuEntity(VUUINT32 flags = 0);

protected:
	// flags
	enum
	{
		CAN_HAVE_CHILDREN	= 1<<0,
		LOCKED_CHILD		= 1<<1,
		GAME_INITIALIZED	= 1<<2,
		IGNORE_REPOSITORY	= 1<<3,
	};

	virtual ~VuEntity();

	// these virtual functions may be be used to customize derived classes
	// (called after the corresponding base method is called)
	virtual void		onLoad(const VuJsonContainer &data)				{}
	virtual void		onPostLoad()									{}
	virtual void		onSave(VuJsonContainer &data) const				{}
	virtual void		onBake()										{}
	virtual void		onClearBaked()									{}
	virtual void		onGameInitialize()								{}
	virtual void		onGameRelease()									{}
	virtual void		onGameReset()									{}

public:
	// configuration (mostly used for editor)
	VUUINT32			canHaveChildren() const							{ return mEntityFlags & CAN_HAVE_CHILDREN; }
	VUUINT32			isLockedChild() const							{ return mEntityFlags & LOCKED_CHILD; }
	VUUINT32			isGameInitialized() const						{ return mEntityFlags & GAME_INITIALIZED; }

	// load/save
	virtual void		load(const VuJsonContainer &data);
	void				postLoad(VUUINT32 parentHash = VU_FNV32_INIT);
	void				save(VuJsonContainer &data) const;

	// bake
	void				bake();
	void				clearBaked();

	// initialize/reset
	void				gameInitialize();
	void				gameRelease();
	void				gameReset();

	// type for creation
	std::string			getCreationType() const;

	// name
	void				setShortName(const std::string &strShortName);
	const std::string	&getShortName() const							{ return mstrShortName; }
	std::string			getLongName() const;
	VUUINT32			getHashedLongName() const;
	VUUINT32			getHashedLongNameFast() const					{ return mHashedLongNameFast; }

	// parent
	VuEntity			*getParentEntity() const						{ return mpParentEntity; }
	void				setParentEntity(VuEntity *pParentEntity);
	bool				isParentOf(const VuEntity *pChildEntity);

	// children
	void				clearChildEntities();
	void				addChildEntity(VuEntity *pEntity);
	bool				removeChildEntity(VuEntity *pEntity);
	void				sortChildEntities();
	int					getChildEntityCount() const						{ return (int)mChildEntities.size(); }
	VuEntity			*getChildEntity(int index) const				{ return mChildEntities[index]; }
	VuEntity			*getChildEntity(const std::string &strShortName) const;

	// root
	VuEntity			*getRootEntity();
	VuEntity			*findEntity(const std::string &strLongName);

	// properties
	VuProperty			*addProperty(VuProperty *pProperty)				{ return mProperties.add(pProperty); }
	void				removeProperty(VuProperty *pProperty)			{ mProperties.remove(pProperty); }
	void				clearProperties()								{ mProperties.clear(); }
	VuProperties		&getProperties()								{ return mProperties; }
	const VuProperties	&getProperties() const							{ return mProperties; }
	VuProperty			*getProperty(const std::string &strName) const;

	// components
	void					addComponent(VuComponent *pComponent)		{ mComponents.add(pComponent); }
	void					removeComponent(VuComponent *pComponent)	{ mComponents.remove(pComponent); }
	template<class T>T		*getComponent() const						{ return mComponents.get<T>(); }
	const VuComponentList	&getComponentList() const					{ return mComponents; }
	VuTransformComponent	*getTransformComponent() const				{ return mpTransformComponent; }

	// events
	void				handleEvent(const char *name, const VuParams &params = VuParams()) const { handleEvent(VuHash::fnv32String(name), params); }
	void				handleEventRecursive(const char *name, const VuParams &params = VuParams()) const { handleEventRecursive(VuHash::fnv32String(name), params); }

	void				handleEvent(VUUINT32 key, const VuParams &params = VuParams()) const;
	virtual void		handleEventRecursive(VUUINT32 key, const VuParams &params = VuParams()) const;

	// templates
	bool					isTemplateRoot() const						{ return mpTemplateAsset ? true : false; }
	bool					isTemplateChild() const;
	void					applyTemplate(VuTemplateAsset *pTemplateAsset);
	const VuJsonContainer	&getTemplateData() const;

protected:
	typedef std::vector<VuEntity *> ChildEntities;

	void				loadChildEntities(const VuJsonContainer &data);
	void				saveChildEntities(VuJsonContainer &data) const;
	void				loadTemplated(const VuJsonContainer &data);
	void				saveTemplated(VuJsonContainer &data) const;
	void				applyTemplateRecursive();

	VUUINT32			mEntityFlags;
	std::string			mstrShortName;
	VUUINT32			mHashedLongNameFast;
	VuEntity			*mpParentEntity;
	ChildEntities		mChildEntities;
	VuProperties		mProperties;
	VuComponentList		mComponents;
	VuTemplateAsset		*mpTemplateAsset;

protected:
	VuTransformComponent	*mpTransformComponent;
};

//*****************************************************************************
// Macro used by the various entity implementations to declare their creation
// functions.
#define IMPLEMENT_ENTITY_REGISTRATION(type) \
	VuEntity *Create##type(const char *strType) { return new type; }
