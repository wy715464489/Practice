//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Property class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Method/VuMethod.h"
#include "VuEngine/Util/VuHash.h"

class VuJsonContainer;
class VuFastContainer;

class VuProperty
{
public:
	enum eType { INTEGER, FLOAT, BOOLEAN, STRING, ENUM_STRING, ENUM_INT, COLOR, VECTOR2D, VECTOR3D, RECT, NOTIFY, ASSET, AUDIO, BLOB, NUM_TYPES };

	VuProperty(const char *strName) : mstrName(strName), mHashedName(VuHash::fnv32String(strName)), mbEnabled(true), mbNotifyOnLoad(false), mpWatcher(VUNULL), mpNextProperty(VUNULL) {}
	virtual ~VuProperty() { delete mpWatcher; }

	virtual eType		getType() const = 0;

	virtual void		load(const VuJsonContainer &data) = 0;
	virtual void		save(VuJsonContainer &data) const = 0;
	virtual void		load(const VuFastContainer &data) = 0;

	virtual void		setCurrent(const VuJsonContainer &data, bool notify = true) = 0;
	virtual void		getCurrent(VuJsonContainer &data) const = 0;

	virtual void		getDefault(VuJsonContainer &data) const = 0;
	virtual void		updateDefault() = 0;

	virtual void		reset() = 0;

	const char			*getName() const	{ return mstrName; }
	VuProperty			*getNextProperty() const { return mpNextProperty; }

	// enabling/disabling
	void				enable(bool bEnable){ mbEnabled = bEnable; }
	bool				isEnabled() const	{ return mbEnabled; }

	void				setNotifyOnLoad() { mbNotifyOnLoad = true; }

	// allows setting a watcher
	template<class T>
	void				setWatcher(T *pObj, void (T::*method)());

protected:
	void				notifyWatcher() { if ( mpWatcher) mpWatcher->execute(); }

	const char					*mstrName;
	VUUINT32					mHashedName;
	bool						mbEnabled;
	bool						mbNotifyOnLoad;
	VuMethodInterface0<void>	*mpWatcher;
	VuProperty					*mpNextProperty;

	friend class VuProperties;
};

//*****************************************************************************
template<class T>
void VuProperty::setWatcher(T *pObj, void (T::*method)())
{
#ifdef VURETAIL
	if ( mbNotifyOnLoad )
	{
		// create event handler
		delete mpWatcher;
		mpWatcher = new VuMethod0<T, void>(pObj, method);
	}
#else
	// create event handler
	delete mpWatcher;
	mpWatcher = new VuMethod0<T, void>(pObj, method);
#endif
}
