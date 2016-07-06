//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuPlayAdditiveAnimationEntity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Assets/VuAnimationAsset.h"


class VuPlayAdditiveAnimationEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuPlayAdditiveAnimationEntity();

	virtual void		onGameRelease();

private:
	VuRetVal			Start(const VuParams &params) { start(); return VuRetVal(); }
	VuRetVal			Stop(const VuParams &params) { stop(); return VuRetVal(); }

	void				start();
	void				stop();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mAnimationAssetName;
	float				mBlendInTime;
	float				mBlendOutTime;
	float				mTimeFactor;
	bool				mLooping;

	// property references
	VuAssetProperty<VuAnimationAsset>	*mpAnimationAssetProperty;

	bool				mPlaying;
};


IMPLEMENT_RTTI(VuPlayAdditiveAnimationEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPlayAdditiveAnimationEntity);


//*****************************************************************************
VuPlayAdditiveAnimationEntity::VuPlayAdditiveAnimationEntity():
	mBlendInTime(0.0f),
	mBlendOutTime(0.0f),
	mTimeFactor(1.0f),
	mLooping(true),
	mPlaying(false)
{
	// properties
	addProperty(mpAnimationAssetProperty = new VuAssetProperty<VuAnimationAsset>("Animation Asset", mAnimationAssetName));
	addProperty(new VuFloatProperty("Blend In Time", mBlendInTime));
	addProperty(new VuFloatProperty("Blend Out Time", mBlendOutTime));
	addProperty(new VuFloatProperty("Time Factor", mTimeFactor));
	addProperty(new VuBoolProperty("Looping", mLooping));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 200));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPlayAdditiveAnimationEntity, Start);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPlayAdditiveAnimationEntity, Stop);
	ADD_SCRIPT_OUTPUT(mpScriptComponent, AddAdditiveAnimation, VuRetVal::Void, VuParamDecl(5, VuParams::UnsignedInt, VuParams::Asset, VuParams::Float, VuParams::Float, VuParams::Bool));
	ADD_SCRIPT_OUTPUT(mpScriptComponent, RemoveAdditiveAnimation, VuRetVal::Void, VuParamDecl(2, VuParams::UnsignedInt, VuParams::Float));
}

//*****************************************************************************
void VuPlayAdditiveAnimationEntity::onGameRelease()
{
	stop();
}

//*****************************************************************************
void VuPlayAdditiveAnimationEntity::start()
{
	if ( !mPlaying )
	{
		if ( VuAsset *pAsset = mpAnimationAssetProperty->getAsset() )
		{
			VuParams params;
			params.addUnsignedInt(getHashedLongNameFast());
			params.addAsset(pAsset);
			params.addFloat(mBlendInTime);
			params.addFloat(mTimeFactor);
			params.addBool(mLooping);
			mpScriptComponent->getPlug("AddAdditiveAnimation")->execute(params);

			mPlaying = true;
		}
	}
}

//*****************************************************************************
void VuPlayAdditiveAnimationEntity::stop()
{
	if ( mPlaying )
	{
		VuParams params;
		params.addUnsignedInt(getHashedLongNameFast());
		params.addFloat(mBlendOutTime);
		mpScriptComponent->getPlug("RemoveAdditiveAnimation")->execute(params);

		mPlaying = false;
	}
}
