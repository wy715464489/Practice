//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuTransitionComponent
// 
//*****************************************************************************

#include "VuTransitionComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
// VuTransitionBaseComponent
//*****************************************************************************

IMPLEMENT_RTTI(VuTransitionBaseComponent, VuComponent);

//*****************************************************************************
VuTransitionBaseComponent::VuTransitionBaseComponent(VuEntity *pOwnerEntity) : VuComponent(pOwnerEntity),
	mTransitionDuration(0.3f),
	mBehavior(BEHAVIOR_AUTOMATIC),
	mState(STATE_OFF)
{
	static VuStaticIntEnumProperty::Choice sBehaviorChoices[] = {
		{ "Automatic", BEHAVIOR_AUTOMATIC },
		{ "Manual In", BEHAVIOR_MANUAL_IN },
		{ "Manual Out", BEHAVIOR_MANUAL_OUT },
		{ VUNULL }
	};

	// properties
	addProperty(new VuFloatProperty("Duration", mTransitionDuration));
	addProperty(new VuStaticIntEnumProperty("Behavior", mBehavior, sBehaviorChoices));

	// scripting
	if ( VuScriptComponent *pSC = pOwnerEntity->getComponent<VuScriptComponent>() )
	{
		ADD_SCRIPT_INPUT_NOARGS(pSC, VuTransitionComponent, TransitionIn);
		ADD_SCRIPT_INPUT_NOARGS(pSC, VuTransitionComponent, TransitionOut);
	}
}



//*****************************************************************************
// VuTransitionComponent
//*****************************************************************************

IMPLEMENT_RTTI(VuTransitionComponent, VuTransitionBaseComponent);

//*****************************************************************************
VuTransitionComponent::VuTransitionComponent(VuEntity *pOwnerEntity) : VuTransitionBaseComponent(pOwnerEntity),
	mTransitionType(TRANS_NONE),
	mHiddenOffset(0,0),
	mAngularFrequency(2.0f),
	mDampingRatio(0.5f),
	mBasePosition(0,0),
	mProgress(0.0f),
	mAlpha(0.0f),
	mSpringTarget(0.0f),
	mSpringPos(0.0f),
	mSpringVel(0.0f),
	mSpringActive(false)
{
	static VuStaticIntEnumProperty::Choice sTransChoices[] = {
		{"None", TRANS_NONE},
		{"Fade", TRANS_FADE},
		{"Offset", TRANS_OFFSET},
		{"Boing", TRANS_BOING},
		{"OffsetBounce", TRANS_OFFSET_BOUNCE},
		{VUNULL}
	};

	// properties
	addProperty(new VuStaticIntEnumProperty("Transition", mTransitionType, sTransChoices));
	addProperty(new VuVector2Property("HiddenOffset", mHiddenOffset));
	addProperty(new VuFloatProperty("AngularFrequency", mAngularFrequency));
	addProperty(new VuFloatProperty("DampingRatio", mDampingRatio));
}

//*****************************************************************************
void VuTransitionComponent::onPostLoad()
{
	VuTransformComponent *pTC = getOwnerEntity()->getTransformComponent();
	mBasePosition.mX = pTC->getLocalPosition().mX;
	mBasePosition.mY = pTC->getLocalPosition().mY;
}

//*****************************************************************************
void VuTransitionComponent::onGameInitialize()
{
	if ( mTransitionType == TRANS_NONE )
	{
		mTransitionDuration = 0.0f;
		mAlpha = 1.0f;
		mState = STATE_ACTIVE;
	}
	else if ( mBehavior == BEHAVIOR_MANUAL_IN )
	{
		setStateActive();
	}
	else if ( mBehavior == BEHAVIOR_MANUAL_OUT )
	{
		setStateOff();
	}
}

//*****************************************************************************
void VuTransitionComponent::tick(float fdt)
{
	if ( mState == STATE_TRANS_IN || mState == STATE_TRANS_OUT )
	{
		float progressStep = (mTransitionDuration > FLT_EPSILON) ? fdt/mTransitionDuration : 1.0f;
		if ( mState == STATE_TRANS_IN )
			mProgress = VuMin(mProgress + progressStep, 1.0f);
		else if ( mState == STATE_TRANS_OUT )
			mProgress = VuMax(mProgress - progressStep, 0.0f);

		if ( mTransitionType == TRANS_FADE )
		{
			mAlpha = mProgress;
		}
		else if ( mTransitionType == TRANS_OFFSET )
		{
			VuTransformComponent *pTC = getOwnerEntity()->getTransformComponent();

			VuVector2 pos = VuLerp(mBasePosition + mHiddenOffset, mBasePosition, mProgress);
			pTC->setLocalPosition(VuVector3(pos.mX, pos.mY, pTC->getLocalPosition().mZ));
			mAlpha = 1.0f;
		}
		else if ( mTransitionType == TRANS_BOING || mTransitionType == TRANS_OFFSET_BOUNCE )
		{
			mSpringActive = true;
			mSpringTarget = (mState == STATE_TRANS_IN) ? 1.0f : 0.0f;
			mAlpha = 1.0f;
		}

		if ( mState == STATE_TRANS_IN && mProgress >= 1.0f )
			mState = STATE_ACTIVE;
		else if ( mState == STATE_TRANS_OUT && mProgress <= 0.0f )
			mState = STATE_OFF;
	}

	if ( mSpringActive )
	{
		VuMathUtil::calcDampedSimpleHarmonicMotion(&mSpringPos, &mSpringVel, mSpringTarget, fdt, mAngularFrequency*VU_2PI, mDampingRatio);

		if ( mSpringTarget <= 0.0f && mSpringPos <= 0.0f )
		{
			mSpringPos = 0.0f;
			mSpringVel = 0.0f;
		}

		if ( VuAbs(mSpringPos - mSpringTarget) < 0.01f && VuAbs(mSpringVel) < 0.01f )
		{
			mSpringActive = false;
			mSpringPos = mSpringTarget;
			mSpringVel = 0.0f;
		}

		if ( mTransitionType == TRANS_BOING )
		{
			VuTransformComponent *pTC = getOwnerEntity()->getTransformComponent();
			float scale = VuMax(mSpringPos, FLT_EPSILON);
			pTC->setLocalScale(VuVector3(scale, scale, 1.0f));
		}
		else if ( mTransitionType == TRANS_OFFSET_BOUNCE )
		{
			VuTransformComponent *pTC = getOwnerEntity()->getTransformComponent();

			VuVector2 pos = VuLerp(mBasePosition + mHiddenOffset, mBasePosition, mSpringPos);
			pTC->setLocalPosition(VuVector3(pos.mX, pos.mY, pTC->getLocalPosition().mZ));
		}
	}
}

//*****************************************************************************
void VuTransitionComponent::transitionIn(bool force)
{
	if ( mTransitionType == TRANS_NONE )
		return;

	if ( mBehavior != BEHAVIOR_AUTOMATIC && !force)
		return;

	setStateOff();
	mState = STATE_TRANS_IN;
}

//*****************************************************************************
void VuTransitionComponent::transitionOut(bool force)
{
	if ( mTransitionType == TRANS_NONE )
		return;

	if ( mBehavior != BEHAVIOR_AUTOMATIC && !force)
		return;

	setStateActive();
	mState = STATE_TRANS_OUT;
}

//*****************************************************************************
void VuTransitionComponent::setStateOff()
{
	mState = STATE_OFF;
	mProgress = 0.0f;
	mAlpha = 0.0f;
	mSpringPos = 0.0f;
	mSpringVel = 0.0f;
}

//*****************************************************************************
void VuTransitionComponent::setStateActive()
{
	mState = STATE_ACTIVE;
	mProgress = 1.0f;
	mAlpha = 1.0f;
	mSpringPos = 1.0f;
	mSpringVel = 0.0f;
}
