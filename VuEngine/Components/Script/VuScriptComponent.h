//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Component class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Math/VuVector2.h"

class VuScriptPlug;
class VuScriptRef;


class VuScriptComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(Script)
	DECLARE_RTTI

public:
	VuScriptComponent(VuEntity *pOwner, int defaultWidth = 150, bool defaultEnabled = true);
	~VuScriptComponent();

	virtual void		onLoad(const VuJsonContainer &data);
	virtual void		onSave(VuJsonContainer &data) const;

	virtual void		onApplyTemplate();

	// adding/removing plugs
	VuScriptPlug		*addPlug(VuScriptPlug *pPlug);
	void				removePlug(const VuScriptPlug *pPlug);
	void				removePlug(int index);

	// adding/removing references
	VuScriptRef			*addRef(VuScriptRef *pRef);
	void				removeRef(const VuScriptRef *pRef);
	void				removeRef(int index);

	// used by the editor
	bool				isEnabled() const					{ return mEnabled; }
	bool				scriptTrace() const					{ return mScriptTrace; }
	int					getWidth() const					{ return mWidth; }
	const VuVector2		&getPosition()const 				{ return mPosition; }
	void				setPosition(const VuVector2 &pos)	{ mPosition = pos; }
	int					getDepth() const					{ return mDepth; }
	void				setDepth(int depth)					{ mDepth = depth; }

	// plug access
	int					getNumPlugs() const			{ return (int)mPlugs.size(); }
	VuScriptPlug		*getPlug(int index) const	{ return mPlugs[index]; }
	VuScriptPlug		*getPlug(const char *strName) const;
	VuScriptPlug		*getPlug(const std::string &strName) const;
	int					getPlugIndex(const VuScriptPlug *pPlug) const;
	int					getNumPlugsOfType(bool bInput) const;

	// ref access
	int					getNumRefs() const			{ return (int)mRefs.size(); }
	VuScriptRef			*getRef(int index) const	{ return mRefs[index]; }
	VuScriptRef			*getRef(const char *strName) const;
	VuScriptRef			*getRef(const std::string &strName) const { return getRef(strName.c_str()); }
	int					getRefIndex(const VuScriptRef *pRef) const;

	// ref connections
	void				addRefConnection(VuScriptRef *pRef);
	void				removeRefConnection(VuScriptRef *pRef);
	bool				isConnectedWith(const VuScriptRef *pRef) const;
	int					getRefConnectionCount() const		{ return (int)mRefConnections.size(); }
	VuScriptRef			*getRefConnection(int index) const	{ return mRefConnections[index]; }

private:
	void				loadRefConnections(const VuJsonContainer &data);
	void				saveRefConnections(VuJsonContainer &data) const;

	typedef std::vector<VuScriptPlug *> Plugs;
	typedef std::vector<VuScriptRef *> Refs;
	typedef std::vector<VuScriptRef *> RefConnections;

	Plugs				mPlugs;
	Refs				mRefs;
	RefConnections		mRefConnections;
	int					mTemplatedRefConnectionCount;
	VuVector2			mTemplatedPosition;

	bool				mEnabled;
	bool				mScriptTrace;
	int					mWidth;
	VuVector2			mPosition;
	int					mDepth;
};


#include "VuScriptPlug.h"
#include "VuScriptRef.h"

#define ADD_SCRIPT_INPUT_NOARGS(component, classname, method) \
	component->addPlug(new VuScriptInputPlug(#method, this, &classname::method))

#define ADD_SCRIPT_INPUT(component, classname, method, retType, paramDecl) \
	component->addPlug(new VuScriptInputPlug(#method, this, &classname::method, retType, paramDecl))

#define ADD_SCRIPT_OUTPUT_NOARGS(component, name) \
	component->addPlug(new VuScriptOutputPlug(#name))

#define ADD_SCRIPT_OUTPUT(component, name, retType, paramDecl) \
	component->addPlug(new VuScriptOutputPlug(#name, retType, paramDecl))

#define ADD_SCRIPT_REF(component, name, refType) \
	component->addRef(new VuScriptRef(#name, refType, component))
