//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PlayAnimation entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuAnimationAsset.h"
#include "VuEngine/Assets/VuTimedEventAsset.h"


class VuPlayAnimationEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuPlayAnimationEntity();

	virtual void		onGameReset();

private:
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mAnimationAssetName;
	std::string			mTimedEventAssetName;
	float				mStartTime;
	float				mBlendTime;
	float				mTimeFactor;
	bool				mLooping;
	bool				mbOneShot;

	// property references
	VuAssetProperty<VuAnimationAsset>	*mpAnimationAssetProperty;
	VuAssetProperty<VuTimedEventAsset>	*mpTimedEventAssetProperty;

	bool				mbIsShot;
};


IMPLEMENT_RTTI(VuPlayAnimationEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPlayAnimationEntity);


//*****************************************************************************
VuPlayAnimationEntity::VuPlayAnimationEntity():
	mStartTime(0.0f),
	mBlendTime(0.0f),
	mTimeFactor(1.0f),
	mLooping(true),
	mbOneShot(false),
	mbIsShot(false)
{
	// properties
	addProperty(mpAnimationAssetProperty = new VuAssetProperty<VuAnimationAsset>("Animation Asset", mAnimationAssetName));
	addProperty(mpTimedEventAssetProperty = new VuAssetProperty<VuTimedEventAsset>("Timed Event Asset", mTimedEventAssetName));
	addProperty(new VuFloatProperty("Start Time", mStartTime));
	addProperty(new VuFloatProperty("Blend Time", mBlendTime));
	addProperty(new VuFloatProperty("Time Factor", mTimeFactor));
	addProperty(new VuBoolProperty("Looping", mLooping));
	addProperty(new VuBoolProperty("One Shot", mbOneShot));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPlayAnimationEntity, Trigger);
	ADD_SCRIPT_OUTPUT(mpScriptComponent, PlayAnimation, VuRetVal::Void, VuParamDecl(5, VuParams::Asset, VuParams::Float, VuParams::Float, VuParams::Float, VuParams::Bool));
}

//*****************************************************************************
void VuPlayAnimationEntity::onGameReset()
{
	mbIsShot = false;
}

//*****************************************************************************
VuRetVal VuPlayAnimationEntity::Trigger(const VuParams &params)
{
	if ( !mbIsShot )
	{
		if ( mbOneShot )
			mbIsShot = true;

		if ( VuAsset *pAsset = mpAnimationAssetProperty->getAsset() )
		{
			VuParams params;
			params.addAsset(pAsset);
			params.addFloat(mStartTime);
			params.addFloat(mBlendTime);
			params.addFloat(mTimeFactor);
			params.addBool(mLooping);
			if ( mpTimedEventAssetProperty->getAsset() )
				params.addAsset(mpTimedEventAssetProperty->getAsset());
			mpScriptComponent->getPlug("PlayAnimation")->execute(params);
		}
	}

	return VuRetVal();
}
