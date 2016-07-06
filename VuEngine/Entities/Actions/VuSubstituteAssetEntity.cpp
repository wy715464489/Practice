//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SubstituteAsset entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAsset.h"


class VuSubstituteAssetEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSubstituteAssetEntity();

	virtual void		onLoad(const VuJsonContainer &data);
	virtual void		onGameInitialize();

private:
	VuRetVal			Trigger(const VuParams &params = VuParams());

	void				typeModified();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mInitiallyActive;
	std::string			mAssetType;
	std::string			mAssetName;
	std::string			mSubstAssetName;

	// property references
	VuBaseAssetProperty	*mpAssetProperty;
	VuBaseAssetProperty	*mpSubstAssetProperty;
};


IMPLEMENT_RTTI(VuSubstituteAssetEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSubstituteAssetEntity);


//*****************************************************************************
VuSubstituteAssetEntity::VuSubstituteAssetEntity():
	mInitiallyActive(true),
	mpAssetProperty(VUNULL),
	mpSubstAssetProperty(VUNULL)
{
	// properties
	addProperty(new VuBoolProperty("Initially Active", mInitiallyActive));
	addProperty(new VuConstStringEnumProperty("Asset Type", mAssetType, VuAssetFactory::IF()->getAssetTypes()))	-> setWatcher(this, &VuSubstituteAssetEntity::typeModified);

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSubstituteAssetEntity, Trigger);
}

//*****************************************************************************
void VuSubstituteAssetEntity::onLoad(const VuJsonContainer &data)
{
	if ( mpAssetProperty == VUNULL || mAssetType != mpAssetProperty->getAssetType() )
		typeModified();

	mpAssetProperty->load(data["Properties"]);
	mpSubstAssetProperty->load(data["Properties"]);
}

//*****************************************************************************
void VuSubstituteAssetEntity::onGameInitialize()
{
	if ( mInitiallyActive )
		Trigger();
}

//*****************************************************************************
VuRetVal VuSubstituteAssetEntity::Trigger(const VuParams &params)
{
	VuAsset *pAsset = mpAssetProperty->getAsset();
	const VuAsset *pSubstAsset = mpSubstAssetProperty->getAsset();

	if ( pAsset && pSubstAsset )
		if ( pAsset != pSubstAsset )
			pAsset->substitute(pSubstAsset);

	return VuRetVal();
}

//*****************************************************************************
void VuSubstituteAssetEntity::typeModified()
{
	removeProperty(mpAssetProperty);
	removeProperty(mpSubstAssetProperty);

	mAssetName = "";
	mSubstAssetName = "";

	addProperty(mpAssetProperty = new VuBaseAssetProperty(mAssetType.c_str(), "Asset Name", mAssetName));
	addProperty(mpSubstAssetProperty = new VuBaseAssetProperty(mAssetType.c_str(), "Subst Asset Name", mSubstAssetName));
}
