//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Component class.  This is the base class for all entity components.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRTTI.h"
#include "VuEngine/Properties/VuProperties.h"

class VuEntity;
class VuJsonContainer;


class VuComponent
{
	DECLARE_RTTI

public:
	VuComponent(VuEntity *pOwnerEntity) : mpOwnerEntity(pOwnerEntity), mpNextComponent(VUNULL) {}
	virtual const char *getShortComponentType() const = 0;
protected:
	virtual ~VuComponent() {}
public:

	// these virtual functions may be be used to customize derived classes
	// (called after the corresponding base method is called)
	virtual void		onLoad(const VuJsonContainer &data)				{}
	virtual void		onPostLoad()									{}
	virtual void		onSave(VuJsonContainer &data) const				{}
	virtual void		onBake()										{}
	virtual void		onClearBaked()									{}
	virtual void		onApplyTemplate()								{}
	virtual void		onGameInitialize()								{}
	virtual void		onGameRelease()									{}
	virtual void		onGameReset()									{}

	void				load(const VuJsonContainer &data);
	void				postLoad();
	void				save(VuJsonContainer &data) const;

	void				bake();
	void				clearBaked();

	void				applyTemplate();

	void				gameInitialize();
	void				gameRelease();
	void				gameReset();

	VuEntity			*getOwnerEntity() const { return mpOwnerEntity; }
	VuComponent			*getNextComponent() const { return mpNextComponent; }

	// properties
	VuProperty			*addProperty(VuProperty *pProperty)		{ return mProperties.add(pProperty); }
	void				removeProperty(VuProperty *pProperty)	{ mProperties.remove(pProperty); }
	void				clearProperties()						{ mProperties.clear(); }
	VuProperties		&getProperties()						{ return mProperties; }
	const VuProperties	&getProperties() const					{ return mProperties; }

private:
	VuProperties		mProperties;
	VuEntity			*mpOwnerEntity;
	VuComponent			*mpNextComponent;

	friend class VuComponentList;
};


//*****************************************************************************
// Macro used to declare RTTI in the class definition.
#define DECLARE_SHORT_COMPONENT_TYPE(type)								\
public:																	\
	virtual const char *getShortComponentType() const { return #type; }	\
private:
