//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Script Plug class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Method/VuMethod.h"
#include "VuEngine/Method/VuParams.h"
#include "VuEngine/Objects/VuRefObj.h"

class VuScriptComponent;
class VuScriptPlug;
class VuEntity;
class VuJsonContainer;


//*****************************************************************************
// script plug base class
//*****************************************************************************
class VuScriptPlug : public VuRefObj
{
public:
	VuScriptPlug(const char *strName, VuRetVal::eType retType, const VuParamDecl &paramDecl);
protected:
	~VuScriptPlug();
public:

	void					setName(const std::string &strName)									{ mstrName = strName; }
	void					setOwnerScriptComponent(VuScriptComponent *ScriptComponent)			{ mpOwnerScriptComponent = ScriptComponent; }
	void					setParamTypes(VuRetVal::eType retType, const VuParamDecl &paramDecl)	{ mRetType = retType; mParamDecl = paramDecl; }

	const std::string		&getName() const					{ return mstrName; }
	VuRetVal::eType			getRetType() const					{ return mRetType; }
	const VuParamDecl		&getParamDecl() const				{ return mParamDecl; }
	VuScriptComponent		*getOwnerScriptComponent()			{ return mpOwnerScriptComponent; }
	const VuScriptComponent	*getOwnerScriptComponent() const	{ return mpOwnerScriptComponent; }

	virtual bool			isInput() const = 0;
	virtual VuRetVal		execute(const VuParams &params = VuParams()) const = 0;

	static bool				areCompatible(const VuScriptPlug &a, const VuScriptPlug &b);
	void					connect(VuScriptPlug &plug);
	void					disconnect(VuScriptPlug &plug);

	int						getNumConnections() const		{ return (int)mConnections.size(); }
	VuScriptPlug			*getConnection(int index) const	{ return mConnections[index]; }

	// load/save
	void					load(const VuJsonContainer &data);
	void					save(VuJsonContainer &data) const;

	// templates
	void					applyTemplate()								{ mTemplatedConnectionCount = (int)mConnections.size(); }

	VuRetVal				execConnections(const VuParams &params) const;

protected:
	void					loadConnections(const VuJsonContainer &data);
	void					saveConnections(VuJsonContainer &data) const;

	std::string					mstrName;
	VuRetVal::eType				mRetType;
	VuParamDecl					mParamDecl;
	VuScriptComponent			*mpOwnerScriptComponent;
	std::vector<VuScriptPlug *>	mConnections;
	int							mTemplatedConnectionCount;

	static bool					smScriptTrace;
};

//*****************************************************************************
// input plug used by script components
// 
//*****************************************************************************
class VuScriptInputPlug : public VuScriptPlug
{
public:
	template <class T>
	VuScriptInputPlug(const char *strName, T *pObj, VuRetVal (T::*method)(const VuParams &params), VuRetVal::eType retType = VuRetVal::Void, const VuParamDecl &paramDecl = VuParamDecl());
	inline ~VuScriptInputPlug();

	virtual bool		isInput() const							{ return true; }
	virtual VuRetVal	execute(const VuParams &params) const	{ return mpMethod->execute(params); }

private:
	VuMethodInterface1<VuRetVal, const VuParams &>	*mpMethod;
};

//*****************************************************************************
// output plug used by script components
//*****************************************************************************
class VuScriptOutputPlug : public VuScriptPlug
{
public:
	VuScriptOutputPlug(const char *strName, VuRetVal::eType retType = VuRetVal::Void, const VuParamDecl &paramDecl = VuParamDecl()):
		VuScriptPlug(strName, retType, paramDecl)
		{}

	virtual bool		isInput() const	{ return false; }
	virtual VuRetVal	execute(const VuParams &params) const;
};


#include "VuScriptPlug.inl"
