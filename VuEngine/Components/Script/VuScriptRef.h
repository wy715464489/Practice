//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Ref class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Objects/VuRefObj.h"

class VuScriptComponent;
class VuJsonContainer;


//*****************************************************************************
// script ref base class
//*****************************************************************************
class VuScriptRef : public VuRefObj
{
public:
	VuScriptRef(const char *strName, const VuRTTI &refType, VuScriptComponent *pOwnerScriptComponent);
protected:
	~VuScriptRef();
public:

	const char				*getName() const					{ return mstrName; }
	const VuRTTI			&getRefType() const					{ return mRefType; }
	VuScriptComponent		*getOwnerScriptComponent()			{ return mpOwnerScriptComponent; }
	const VuScriptComponent	*getOwnerScriptComponent() const	{ return mpOwnerScriptComponent; }

	bool					isCompatibleWith(const VuEntity *pEntity) const;
	void					connect(VuScriptComponent &script);
	void					disconnect();

	VuScriptComponent		*getRefScript() const	{ return mpRefScriptComponent; }
	VuEntity				*getRefEntity() const;
	template<class T>T		*getRefEntity() const;

	// load/save
	void					load(const VuJsonContainer &data);
	void					save(VuJsonContainer &data) const;

	// templates
	void					applyTemplate()								{ mbTemplatedRef = mpRefScriptComponent ? true : false; }

	template<class T>
	void					setWatcher(T *pObj, void (T::*method)()) { mpWatcher = new VuMethod0<T, void>(static_cast<T *>(pObj), method); }

private:
	const char				*mstrName;
	const VuRTTI			&mRefType;
	VuScriptComponent		*mpOwnerScriptComponent;
	VuScriptComponent		*mpRefScriptComponent;
	bool					mbTemplatedRef;

	VuMethodInterface0<void>	*mpWatcher;
};


#include "VuScriptRef.inl"
