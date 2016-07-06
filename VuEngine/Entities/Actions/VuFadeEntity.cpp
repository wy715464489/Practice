//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Attach entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuTickManager.h"


class VuFadeEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuFadeEntity();

	virtual void		onGameRelease();

protected:
	VuRetVal			StartFadeIn(const VuParams &params);
	VuRetVal			StartFadeOut(const VuParams &params);

	void				tickDecision(float fdt);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	float				mDuration;

	// plubs
	VuScriptPlug		*mpSetAlphaPlug;
	VuScriptPlug		*mpDonePlug;

	enum eState { IDLE, FADE_IN, FADE_OUT };
	eState				mState;
	float				mTimer;
};

IMPLEMENT_RTTI(VuFadeEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuFadeEntity);


//*****************************************************************************
VuFadeEntity::VuFadeEntity():
	mDuration(1.0f),
	mState(IDLE)
{
	// properties
	addProperty(new VuFloatProperty("Duration", mDuration));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuFadeEntity, StartFadeIn);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuFadeEntity, StartFadeOut);
	mpSetAlphaPlug = ADD_SCRIPT_OUTPUT(mpScriptComponent, SetAlpha, VuRetVal::Void, VuParamDecl(1, VuParams::Float));
	mpDonePlug = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Done);
}

//*****************************************************************************
void VuFadeEntity::onGameRelease()
{
	VuTickManager::IF()->unregisterHandler(this, "Decision");
}

//*****************************************************************************
VuRetVal VuFadeEntity::StartFadeIn(const VuParams &params)
{
	if ( mState == IDLE )
	{
		mState = FADE_IN;
		mTimer = 0.0f;

		VuParams outParams;
		outParams.addFloat(0.0f);
		mpSetAlphaPlug->execute(outParams);

		VuTickManager::IF()->registerHandler(this, &VuFadeEntity::tickDecision, "Decision");
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuFadeEntity::StartFadeOut(const VuParams &params)
{
	if ( mState == IDLE )
	{
		mState = FADE_OUT;
		mTimer = 0.0f;

		VuParams outParams;
		outParams.addFloat(1.0f);
		mpSetAlphaPlug->execute(outParams);

		VuTickManager::IF()->registerHandler(this, &VuFadeEntity::tickDecision, "Decision");
	}

	return VuRetVal();
}

//*****************************************************************************
void VuFadeEntity::tickDecision(float fdt)
{
	float alpha = 1.0f;
	if ( mState == FADE_IN )
	{
		if ( mTimer >= mDuration )
		{
			mState = IDLE;
			mpDonePlug->execute();
			VuTickManager::IF()->unregisterHandler(this, "Decision");
			alpha = 1.0f;
		}
		else
		{
			alpha = mTimer/mDuration;
		}
	}
	else if ( mState == FADE_OUT )
	{
		if ( mTimer >= mDuration )
		{
			mState = IDLE;
			mpDonePlug->execute();
			VuTickManager::IF()->unregisterHandler(this, "Decision");
			alpha = 0.0f;
		}
		else
		{
			alpha = 1.0f - mTimer/mDuration;
		}
	}

	VuParams params;
	params.addFloat(alpha);
	mpSetAlphaPlug->execute(params);

	mTimer += fdt;
}
