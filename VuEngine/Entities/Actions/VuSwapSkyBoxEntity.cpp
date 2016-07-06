//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  SwapSkybox entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuStaticModelAsset.h"
#include "VuEngine/Events/VuEventManager.h"


class VuSwapSkyboxEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSwapSkyboxEntity();

private:
	VuRetVal							Trigger(const VuParams &params);

	// components
	VuScriptComponent					*mpScriptComponent;

	// properties
	std::string							mModelAssetName;

	// property references
	VuAssetProperty<VuStaticModelAsset>	*mpModelAssetProperty;
};


IMPLEMENT_RTTI(VuSwapSkyboxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSwapSkyboxEntity);


//*****************************************************************************
VuSwapSkyboxEntity::VuSwapSkyboxEntity()
{
	// properties
	addProperty(mpModelAssetProperty = new VuAssetProperty<VuStaticModelAsset>("Model Asset", mModelAssetName));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSwapSkyboxEntity, Trigger);
}

//*****************************************************************************
VuRetVal VuSwapSkyboxEntity::Trigger(const VuParams &params)
{
	VuParams outParams;
	outParams.addInt(0);
	outParams.addAsset(mpModelAssetProperty->getAsset());

	VuEventManager::IF()->broadcast("OnSwapSkybox", outParams);

	return VuRetVal();
}