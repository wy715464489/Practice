//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Persistent data entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Managers/VuProfileManager.h"


class VuPersistentBaseEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuPersistentBaseEntity();

protected:
	const VuJsonContainer	&dataRead() const;
	VuJsonContainer			&dataWrite();

	// properties
	bool				mSave;
	bool				mCloudSave;
	std::string			mVariableName;

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuPersistentBaseEntity, VuEntity);


//*****************************************************************************
VuPersistentBaseEntity::VuPersistentBaseEntity():
	mSave(false),
	mCloudSave(false)
{
	// properties
	addProperty(new VuBoolProperty("Save", mSave));
	addProperty(new VuBoolProperty("Cloud Save", mCloudSave));
	addProperty(new VuStringProperty("Name", mVariableName));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));
}

//*****************************************************************************
const VuJsonContainer &VuPersistentBaseEntity::dataRead() const
{
	if ( mCloudSave )
		return VuProfileManager::IF()->dataRead()["PersistentCloudData"][mVariableName];

	if ( mSave )
		return VuProfileManager::IF()->dataRead()["PersistentData"][mVariableName];

	return VuProfileManager::IF()->tempDataRead()["PersistentData"][mVariableName];
}

//*****************************************************************************
VuJsonContainer &VuPersistentBaseEntity::dataWrite()
{
	if ( mCloudSave )
		return VuProfileManager::IF()->dataWrite()["PersistentCloudData"][mVariableName];

	if ( mSave )
		return VuProfileManager::IF()->dataWrite()["PersistentData"][mVariableName];

	return VuProfileManager::IF()->tempDataWrite()["PersistentData"][mVariableName];
}


//*****************************************************************************
// Boolean
//*****************************************************************************
class VuPersistentBooleanEntity : public VuPersistentBaseEntity
{
	DECLARE_RTTI

public:
	VuPersistentBooleanEntity();

private:
	VuRetVal			Set(const VuParams &params);
	VuRetVal			SetTrue(const VuParams &params)		{ set(true); return VuRetVal(); }
	VuRetVal			SetFalse(const VuParams &params)	{ set(false); return VuRetVal(); }
	VuRetVal			Get(const VuParams &params)			{ return VuRetVal(get()); }

	void				set(bool bValue);
	bool				get();
};

IMPLEMENT_RTTI(VuPersistentBooleanEntity, VuPersistentBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPersistentBooleanEntity);


//*****************************************************************************
VuPersistentBooleanEntity::VuPersistentBooleanEntity()
{
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentBooleanEntity, Set, VuRetVal::Void, VuParamDecl(1, VuParams::Bool));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentBooleanEntity, SetTrue, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentBooleanEntity, SetFalse, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentBooleanEntity, Get, VuRetVal::Bool, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuPersistentBooleanEntity::Set(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	set(accessor.getBool());

	return VuRetVal();
}

//*****************************************************************************
void VuPersistentBooleanEntity::set(bool bValue)
{
	if ( mVariableName.size() )
		dataWrite().putValue(bValue);
}

//*****************************************************************************
bool VuPersistentBooleanEntity::get()
{
	return dataRead().asBool();
}


//*****************************************************************************
// Integer
//*****************************************************************************
class VuPersistentIntegerEntity : public VuPersistentBaseEntity
{
	DECLARE_RTTI

public:
	VuPersistentIntegerEntity();

private:
	VuRetVal			Set(const VuParams &params);
	VuRetVal			Get(const VuParams &params);
};

IMPLEMENT_RTTI(VuPersistentIntegerEntity, VuPersistentBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPersistentIntegerEntity);


//*****************************************************************************
VuPersistentIntegerEntity::VuPersistentIntegerEntity()
{
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentIntegerEntity, Set, VuRetVal::Void, VuParamDecl(1, VuParams::Int));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentIntegerEntity, Get, VuRetVal::Int, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuPersistentIntegerEntity::Set(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	if ( mVariableName.size() )
		dataWrite().putValue(accessor.getInt());

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuPersistentIntegerEntity::Get(const VuParams &params)
{
	return VuRetVal(dataRead().asInt());
}


//*****************************************************************************
// String
//*****************************************************************************
class VuPersistentStringEntity : public VuPersistentBaseEntity
{
	DECLARE_RTTI

public:
	VuPersistentStringEntity();

private:
	VuRetVal			Set(const VuParams &params);
	VuRetVal			Get(const VuParams &params);
};

IMPLEMENT_RTTI(VuPersistentStringEntity, VuPersistentBaseEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPersistentStringEntity);


//*****************************************************************************
VuPersistentStringEntity::VuPersistentStringEntity()
{
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentStringEntity, Set, VuRetVal::Void, VuParamDecl(1, VuParams::String));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuPersistentStringEntity, Get, VuRetVal::String, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuPersistentStringEntity::Set(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	if ( mVariableName.size() )
		dataWrite().putValue(accessor.getString());

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuPersistentStringEntity::Get(const VuParams &params)
{
	return VuRetVal(dataRead().asCString());
}
