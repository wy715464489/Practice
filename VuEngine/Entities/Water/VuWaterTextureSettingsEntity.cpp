//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  WaterTextureSettings entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuNotifyProperty.h"
#include "VuEngine/Water/VuWaterRenderer.h"
#include "VuEngine/Water/VuWater.h"


class VuWaterTextureSettingsEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuWaterTextureSettingsEntity();

private:
	VuRetVal			Trigger(const VuParams &params)	{ use(); return VuRetVal(); }

	void				use();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	VuWaterRendererTextureDesc	mWaterTextureDesc;
};


IMPLEMENT_RTTI(VuWaterTextureSettingsEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuWaterTextureSettingsEntity);


//*****************************************************************************
VuWaterTextureSettingsEntity::VuWaterTextureSettingsEntity()
{
	// properties
	addProperty(new VuFloatProperty("Gravity", mWaterTextureDesc.mGravity));
	addProperty(new VuFloatProperty("WorldSize", mWaterTextureDesc.mWorldSize));
	addProperty(new VuFloatProperty("WindSpeed", mWaterTextureDesc.mWindSpeed));
	addProperty(new VuFloatProperty("DirectionalPower", mWaterTextureDesc.mDirectionalPower));
	addProperty(new VuFloatProperty("SuppressionWaveLength", mWaterTextureDesc.mSuppressionWaveLength));
	addProperty(new VuFloatProperty("HeightFactor", mWaterTextureDesc.mHeightFactor));
	addProperty(new VuFloatProperty("TimeFactor", mWaterTextureDesc.mTimeFactor));
	addProperty(new VuFloatProperty("NormalTextureScale", mWaterTextureDesc.mNormalTextureScale));

	addProperty(new VuNotifyProperty("Use")) -> setWatcher(this, &VuWaterTextureSettingsEntity::use);

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuWaterTextureSettingsEntity, Trigger);
}

//*****************************************************************************
void VuWaterTextureSettingsEntity::use()
{
	if ( VuWater::IF() )
	{
		VuWater::IF()->renderer()->setWaterTextureDesc(mWaterTextureDesc);
	}
}
