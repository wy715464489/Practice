//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Logic entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuLicenseManager.h"
#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/Math/VuRand.h"


//*****************************************************************************
// If
//*****************************************************************************
class VuIfEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuIfEntity();

private:
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuIfEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuIfEntity);


//*****************************************************************************
VuIfEntity::VuIfEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuIfEntity, Trigger);
	ADD_SCRIPT_OUTPUT(mpScriptComponent, Operand, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, True, VuRetVal::Void, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, False, VuRetVal::Void, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuIfEntity::Trigger(const VuParams &params)
{
	VuRetVal result = mpScriptComponent->getPlug("Operand")->execute();
	if ( result.getType() == VuRetVal::Bool )
	{
		if ( result.asBool() )
			mpScriptComponent->getPlug("True")->execute(params);
		else
			mpScriptComponent->getPlug("False")->execute(params);
	}

	return VuRetVal();
}


//*****************************************************************************
// Compare Booleans
//*****************************************************************************
class VuCompareBooleansEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCompareBooleansEntity();

private:
	VuRetVal			Equal(const VuParams &params)	{ return VuRetVal(get("A") == get("B")); }

	bool				get(const char *operand);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuCompareBooleansEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCompareBooleansEntity);

//*****************************************************************************
VuCompareBooleansEntity::VuCompareBooleansEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	mpScriptComponent->addPlug(new VuScriptInputPlug("A == B", this, &VuCompareBooleansEntity::Equal, VuRetVal::Bool));
	ADD_SCRIPT_OUTPUT(mpScriptComponent, A, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, B, VuRetVal::Bool, VuParamDecl());
}

//*****************************************************************************
bool VuCompareBooleansEntity::get(const char *operand)
{
	VuRetVal result = mpScriptComponent->getPlug(operand)->execute();
	if ( result.getType() == VuRetVal::Int )
		return result.asBool();
	return false;
}


//*****************************************************************************
// Compare Integers
//*****************************************************************************
class VuCompareIntegersEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCompareIntegersEntity();

private:
	VuRetVal			Greater(const VuParams &params)      { return VuRetVal(get("A") > get("B")); }
	VuRetVal			GreaterEqual(const VuParams &params) { return VuRetVal(get("A") >= get("B")); }
	VuRetVal			Less(const VuParams &params)         { return VuRetVal(get("A") < get("B")); }
	VuRetVal			LessEqual(const VuParams &params)    { return VuRetVal(get("A") <= get("B")); }
	VuRetVal			Equal(const VuParams &params)        { return VuRetVal(get("A") == get("B")); }
	VuRetVal			NotEqual(const VuParams &params)     { return VuRetVal(get("A") != get("B")); }

	int					get(const char *operand);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuCompareIntegersEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCompareIntegersEntity);

//*****************************************************************************
VuCompareIntegersEntity::VuCompareIntegersEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	mpScriptComponent->addPlug(new VuScriptInputPlug("A > B", this, &VuCompareIntegersEntity::Greater, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("A >= B", this, &VuCompareIntegersEntity::GreaterEqual, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("A < B", this, &VuCompareIntegersEntity::Less, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("A <= B", this, &VuCompareIntegersEntity::LessEqual, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("A == B", this, &VuCompareIntegersEntity::Equal, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("A != B", this, &VuCompareIntegersEntity::NotEqual, VuRetVal::Bool));
	ADD_SCRIPT_OUTPUT(mpScriptComponent, A, VuRetVal::Int, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, B, VuRetVal::Int, VuParamDecl());
}

//*****************************************************************************
int VuCompareIntegersEntity::get(const char *operand)
{
	VuRetVal result = mpScriptComponent->getPlug(operand)->execute();
	if ( result.getType() == VuRetVal::Int )
		return result.asInt();
	return 0;
}


//*****************************************************************************
// Compare Integer To Constant
//*****************************************************************************
class VuCompareIntegerToConstantEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCompareIntegerToConstantEntity();

private:
	VuRetVal			Greater(const VuParams &params)      { return VuRetVal(getInteger() > mConstant); }
	VuRetVal			GreaterEqual(const VuParams &params) { return VuRetVal(getInteger() >= mConstant); }
	VuRetVal			Less(const VuParams &params)         { return VuRetVal(getInteger() < mConstant); }
	VuRetVal			LessEqual(const VuParams &params)    { return VuRetVal(getInteger() <= mConstant); }
	VuRetVal			Equal(const VuParams &params)        { return VuRetVal(getInteger() == mConstant); }

	int					getInteger();

	// components
	VuScriptComponent	*mpScriptComponent;
	
	// properties
	int					mConstant;

	// plugs
	VuScriptPlug		*mpIntegerPlug;
};

IMPLEMENT_RTTI(VuCompareIntegerToConstantEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCompareIntegerToConstantEntity);

//*****************************************************************************
VuCompareIntegerToConstantEntity::VuCompareIntegerToConstantEntity():
	mConstant(0)
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	addProperty(new VuIntProperty("Constant", mConstant));

	mpScriptComponent->addPlug(new VuScriptInputPlug("Greater", this, &VuCompareIntegerToConstantEntity::Greater, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("GreaterEqual", this, &VuCompareIntegerToConstantEntity::GreaterEqual, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("Less", this, &VuCompareIntegerToConstantEntity::Less, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("LessEqual", this, &VuCompareIntegerToConstantEntity::LessEqual, VuRetVal::Bool));
	mpScriptComponent->addPlug(new VuScriptInputPlug("Equal", this, &VuCompareIntegerToConstantEntity::Equal, VuRetVal::Bool));
	mpIntegerPlug = ADD_SCRIPT_OUTPUT(mpScriptComponent, Integer, VuRetVal::Int, VuParamDecl());
}

//*****************************************************************************
int VuCompareIntegerToConstantEntity::getInteger()
{
	VuRetVal result = mpIntegerPlug->execute();
	if ( result.getType() == VuRetVal::Int )
		return result.asInt();
	return 0;
}


//*****************************************************************************
// Compare Strings
//*****************************************************************************
class VuCompareStringsEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCompareStringsEntity();

private:
	VuRetVal			Equal(const VuParams &params)	{ return VuRetVal(strcmp(get("A"), get("B")) == 0); }

	const char			*get(const char *operand);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuCompareStringsEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCompareStringsEntity);

//*****************************************************************************
VuCompareStringsEntity::VuCompareStringsEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	mpScriptComponent->addPlug(new VuScriptInputPlug("A == B", this, &VuCompareStringsEntity::Equal, VuRetVal::Bool));
	ADD_SCRIPT_OUTPUT(mpScriptComponent, A, VuRetVal::String, VuParamDecl());
	ADD_SCRIPT_OUTPUT(mpScriptComponent, B, VuRetVal::String, VuParamDecl());
}

//*****************************************************************************
const char *VuCompareStringsEntity::get(const char *operand)
{
	VuRetVal result = mpScriptComponent->getPlug(operand)->execute();
	if ( result.getType() == VuRetVal::String )
		return result.asString();
	return "";
}


//*****************************************************************************
// OneShotFilter
//*****************************************************************************
class VuOneShotFilterEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuOneShotFilterEntity();

private:
	VuRetVal			In(const VuParams &params);
	VuRetVal			Reset(const VuParams &params)	{ mbDone = false; return VuRetVal(); }
	VuRetVal			SetShot(const VuParams &params)	{ mbDone = true; return VuRetVal(); }

	// components
	VuScriptComponent	*mpScriptComponent;

	bool				mbDone;
};

IMPLEMENT_RTTI(VuOneShotFilterEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuOneShotFilterEntity);


//*****************************************************************************
VuOneShotFilterEntity::VuOneShotFilterEntity():
	mbDone(false)
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuOneShotFilterEntity, In);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuOneShotFilterEntity, Reset);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuOneShotFilterEntity, SetShot);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Out);
}

//*****************************************************************************
VuRetVal VuOneShotFilterEntity::In(const VuParams &params)
{
	if ( !mbDone )
	{
		mpScriptComponent->getPlug("Out")->execute(params);
		mbDone = true;
	}

	return VuRetVal();
}


//*****************************************************************************
// Sequencer
//*****************************************************************************
class VuSequencerEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSequencerEntity();

private:
	VuRetVal			In(const VuParams &params);
	VuRetVal			Reset(const VuParams &params)	{ mCount = 0; return VuRetVal(); }

	// components
	VuScriptComponent	*mpScriptComponent;

	int					mCount;
};

IMPLEMENT_RTTI(VuSequencerEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSequencerEntity);


//*****************************************************************************
VuSequencerEntity::VuSequencerEntity():
	mCount(0)
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSequencerEntity, In);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuSequencerEntity, Reset);

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 1);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 2);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 3);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 4);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 5);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 6);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 7);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 8);
}

//*****************************************************************************
VuRetVal VuSequencerEntity::In(const VuParams &params)
{
	if ( mCount < 8 )
	{
		char str[256];
		VU_SPRINTF(str, sizeof(str), "%d", mCount + 1);
		if ( VuScriptPlug *pPlug = mpScriptComponent->getPlug(str) )
			pPlug->execute(params);
		mCount++;
	}

	return VuRetVal();
}


//*****************************************************************************
// Delay
//*****************************************************************************
class VuDelayEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuDelayEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	VuRetVal			In(const VuParams &params);

	void				tickDecision(float fdt);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	float				mDelay;
	float				mRandomWindow;
	bool				mUseRealTime;

	bool				mbActive;
	float				mTimer;
	VuParams			mParams;
};

IMPLEMENT_RTTI(VuDelayEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDelayEntity);


//*****************************************************************************
VuDelayEntity::VuDelayEntity():
	mDelay(1.0f),
	mRandomWindow(0.0f),
	mUseRealTime(false),
	mbActive(false)
{
	addProperty(new VuFloatProperty("Delay", mDelay));
	addProperty(new VuFloatProperty("Random Window", mRandomWindow));
	addProperty(new VuBoolProperty("Use Real Time", mUseRealTime));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDelayEntity, In);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Out);
}

//*****************************************************************************
void VuDelayEntity::onGameInitialize()
{
	mbActive = false;

	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &VuDelayEntity::tickDecision, "Decision");
}

//*****************************************************************************
void VuDelayEntity::onGameRelease()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
VuRetVal VuDelayEntity::In(const VuParams &params)
{
	if ( !mbActive )
	{
		mbActive = true;
		mTimer = mDelay + mRandomWindow*VuRand::global().rand();
		mParams = VuParams();
	}

	return VuRetVal();
}

//*****************************************************************************
void VuDelayEntity::tickDecision(float fdt)
{
	if ( mbActive )
	{
		if ( mUseRealTime )
			fdt = VuTickManager::IF()->getRealDeltaTime();

		mTimer -= fdt;
		if ( mTimer < 0 )
		{
			mbActive = false;
			mpScriptComponent->getPlug("Out")->execute(mParams);
		}
	}
}


//*****************************************************************************
// MultiDelay
//*****************************************************************************
class VuMultiDelayEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuMultiDelayEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	VuRetVal			In(const VuParams &params);

	void				tickDecision(float fdt);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	float				mDelays[8];
	bool				mUseRealTime;

	// plugs
	VuScriptPlug		*mpOutputPlugs[8];

	bool				mbActive;
	float				mTimer;
	VuParams			mParams;
};

IMPLEMENT_RTTI(VuMultiDelayEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuMultiDelayEntity);


//*****************************************************************************
VuMultiDelayEntity::VuMultiDelayEntity():
	mUseRealTime(false),
	mbActive(false)
{
	memset(mDelays, 0, sizeof(mDelays));
	memset(mpOutputPlugs, 0, sizeof(mpOutputPlugs));

	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	addProperty(new VuFloatProperty("Delay 1", mDelays[0]));
	addProperty(new VuFloatProperty("Delay 2", mDelays[1]));
	addProperty(new VuFloatProperty("Delay 3", mDelays[2]));
	addProperty(new VuFloatProperty("Delay 4", mDelays[3]));
	addProperty(new VuFloatProperty("Delay 5", mDelays[4]));
	addProperty(new VuFloatProperty("Delay 6", mDelays[5]));
	addProperty(new VuFloatProperty("Delay 7", mDelays[6]));
	addProperty(new VuFloatProperty("Delay 8", mDelays[7]));
	addProperty(new VuBoolProperty("Use Real Time", mUseRealTime));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuMultiDelayEntity, In);

	mpOutputPlugs[0] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 1);
	mpOutputPlugs[1] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 2);
	mpOutputPlugs[2] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 3);
	mpOutputPlugs[3] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 4);
	mpOutputPlugs[4] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 5);
	mpOutputPlugs[5] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 6);
	mpOutputPlugs[6] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 7);
	mpOutputPlugs[7] = ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 8);
}

//*****************************************************************************
void VuMultiDelayEntity::onGameInitialize()
{
	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &VuMultiDelayEntity::tickDecision, "Decision");
}

//*****************************************************************************
void VuMultiDelayEntity::onGameRelease()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
VuRetVal VuMultiDelayEntity::In(const VuParams &params)
{
	if ( !mbActive )
	{
		mbActive = true;
		mTimer = -FLT_EPSILON;
		mParams = VuParams();
	}

	return VuRetVal();
}

//*****************************************************************************
void VuMultiDelayEntity::tickDecision(float fdt)
{
	if ( mbActive )
	{
		if ( mUseRealTime )
			fdt = VuTickManager::IF()->getRealDeltaTime();

		float nextTime = mTimer + fdt;

		mbActive = false;
		for ( int i = 0; i < 8; i++ )
		{
			if ( mTimer < mDelays[i] )
			{
				if ( nextTime >= mDelays[i] )
					mpOutputPlugs[i]->execute(mParams);
				else
					mbActive = true;
			}
		}

		mTimer = nextTime;
	}
}


//*****************************************************************************
// TriggerArray
//*****************************************************************************
class VuTriggerArrayEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuTriggerArrayEntity();

private:
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuTriggerArrayEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuTriggerArrayEntity);


//*****************************************************************************
VuTriggerArrayEntity::VuTriggerArrayEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuTriggerArrayEntity, Trigger);

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 1);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 2);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 3);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 4);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 5);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 6);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 7);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, 8);
}

//*****************************************************************************
VuRetVal VuTriggerArrayEntity::Trigger(const VuParams &params)
{
	char str[256];

	for ( int i = 0; i < 8; i++ )
	{
		VU_SPRINTF(str, sizeof(str), "%d", i + 1);
		if ( VuScriptPlug *pPlug = mpScriptComponent->getPlug(str) )
			pPlug->execute(params);
	}

	return VuRetVal();
}


//*****************************************************************************
// TriggerForwarder
//*****************************************************************************
class VuTriggerForwarderEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuTriggerForwarderEntity();

private:
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuTriggerForwarderEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuTriggerForwarderEntity);


//*****************************************************************************
VuTriggerForwarderEntity::VuTriggerForwarderEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuTriggerForwarderEntity, Trigger);

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnTrigger);
}

//*****************************************************************************
VuRetVal VuTriggerForwarderEntity::Trigger(const VuParams &params)
{
	if ( VuScriptPlug *pPlug = mpScriptComponent->getPlug("OnTrigger") )
		pPlug->execute(params);

	return VuRetVal();
}


//*****************************************************************************
// Randomize
//*****************************************************************************
class VuRandomTriggerEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuRandomTriggerEntity();

private:
	VuRetVal			Trigger(const VuParams &params);

	enum { MAX_OUTPUTS = 8 };

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mNoRepeat;
	float				mWeights[MAX_OUTPUTS];

	int					mLastPlug;
};

IMPLEMENT_RTTI(VuRandomTriggerEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuRandomTriggerEntity);


//*****************************************************************************
VuRandomTriggerEntity::VuRandomTriggerEntity():
	mNoRepeat(false),
	mLastPlug(-1)
{
	char str[256];

	memset(mWeights, 0, sizeof(mWeights));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// properties
	addProperty(new VuBoolProperty("No Repeat", mNoRepeat));
	addProperty(new VuFloatProperty("Weight 1", mWeights[0]));
	addProperty(new VuFloatProperty("Weight 2", mWeights[1]));
	addProperty(new VuFloatProperty("Weight 3", mWeights[2]));
	addProperty(new VuFloatProperty("Weight 4", mWeights[3]));
	addProperty(new VuFloatProperty("Weight 5", mWeights[4]));
	addProperty(new VuFloatProperty("Weight 6", mWeights[5]));
	addProperty(new VuFloatProperty("Weight 7", mWeights[6]));
	addProperty(new VuFloatProperty("Weight 8", mWeights[7]));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuRandomTriggerEntity, Trigger);

	for ( int i = 0; i < MAX_OUTPUTS; i++ )
	{
		VU_SPRINTF(str, sizeof(str), "%d", i + 1);
		mpScriptComponent->addPlug(new VuScriptOutputPlug(str));
	}
}

//*****************************************************************************
VuRetVal VuRandomTriggerEntity::Trigger(const VuParams &params)
{
	float weights[MAX_OUTPUTS];
	VU_MEMCPY(weights, sizeof(weights), mWeights, sizeof(mWeights));
	if ( mNoRepeat && mLastPlug >= 0 )
		weights[mLastPlug] = 0.0f;

	float totalWeight = 0.0f;
	for ( int i = 0; i < MAX_OUTPUTS; i++ )
		totalWeight += weights[i];

	if ( totalWeight > 0 )
	{
		float weight = totalWeight*VuRand::global().rand();
		for ( int i = 0; i < MAX_OUTPUTS; i++ )
		{
			if ( weight <= weights[i] )
			{
				char str[256];
				VU_SPRINTF(str, sizeof(str), "%d", i + 1);
				if ( VuScriptPlug *pPlug = mpScriptComponent->getPlug(str) )
				{
					mLastPlug = i;
					return pPlug->execute(params);
				}
				break;
			}
			weight -= weights[i];
		}
	}

	return VuRetVal();
}


//*****************************************************************************
// PlatformFilter
//*****************************************************************************
class VuPlatformFilterEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuPlatformFilterEntity();

private:
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuPlatformFilterEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPlatformFilterEntity);


//*****************************************************************************
VuPlatformFilterEntity::VuPlatformFilterEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPlatformFilterEntity, Trigger);

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Win32);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Android);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Ios);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Windows);
}

//*****************************************************************************
VuRetVal VuPlatformFilterEntity::Trigger(const VuParams &params)
{
	if ( VuScriptPlug *pPlug = mpScriptComponent->getPlug(VUPLATFORM) )
		pPlug->execute(params);

	return VuRetVal();
}


//*****************************************************************************
// BuildFilter
//*****************************************************************************
class VuBuildFilterEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuBuildFilterEntity();

private:
	VuRetVal			Trigger(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuBuildFilterEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuBuildFilterEntity);


//*****************************************************************************
VuBuildFilterEntity::VuBuildFilterEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuBuildFilterEntity, Trigger);

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Debug);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Release);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Retail);
}

//*****************************************************************************
VuRetVal VuBuildFilterEntity::Trigger(const VuParams &params)
{
	#if defined VUDEBUG
		mpScriptComponent->getPlug("Debug")->execute(params);
	#elif defined VURELEASE
		mpScriptComponent->getPlug("Release")->execute(params);
	#elif defined VURETAIL
		mpScriptComponent->getPlug("Retail")->execute(params);
	#endif

	return VuRetVal();
}


//*****************************************************************************
// SysCaps
//*****************************************************************************
class VuSysCapsEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuSysCapsEntity();

private:
	VuRetVal			HasTouch(const VuParams &params) { return VuRetVal(VuSys::IF()->hasTouch()); }
	VuRetVal			HasAccel(const VuParams &params) { return VuRetVal(VuSys::IF()->hasAccel()); }
	VuRetVal			HasKeyboard(const VuParams &params) { return VuRetVal(VuSys::IF()->hasKeyboard()); }
	VuRetVal			HasMouse(const VuParams &params) { return VuRetVal(VuSys::IF()->hasMouse()); }
	VuRetVal			HasGamePad(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuSysCapsEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuSysCapsEntity);


//*****************************************************************************
VuSysCapsEntity::VuSysCapsEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT(mpScriptComponent, VuSysCapsEntity, HasTouch, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuSysCapsEntity, HasAccel, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuSysCapsEntity, HasKeyboard, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuSysCapsEntity, HasMouse, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuSysCapsEntity, HasGamePad, VuRetVal::Bool, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuSysCapsEntity::HasGamePad(const VuParams &params)
{
	bool hasGamePad = false;

	for ( int i = 0; i < VuGamePad::MAX_NUM_PADS; i++ )
		hasGamePad |= VuGamePad::IF()->getController(i).mIsConnected;

	return VuRetVal(hasGamePad);
}


//*****************************************************************************
// License
//*****************************************************************************
class VuLicenseEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuLicenseEntity();

private:
	VuRetVal			IsTrial(const VuParams &params) { return VuRetVal(VuLicenseManager::IF()->isTrial()); }

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuLicenseEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuLicenseEntity);


//*****************************************************************************
VuLicenseEntity::VuLicenseEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	ADD_SCRIPT_INPUT(mpScriptComponent, VuLicenseEntity, IsTrial, VuRetVal::Bool, VuParamDecl());
}


//*****************************************************************************
// VuCapacitorEntity
//*****************************************************************************
class VuCapacitorEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCapacitorEntity();

private:
	VuRetVal			In(const VuParams &params);
	VuRetVal			Reset(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	int					mCapacity;

	int					mCharge;
};

IMPLEMENT_RTTI(VuCapacitorEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCapacitorEntity);


//*****************************************************************************
VuCapacitorEntity::VuCapacitorEntity():
	mCapacity(5),
	mCharge(0)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// properties
	addProperty(new VuIntProperty("Capacity", mCapacity));

	// scripting

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCapacitorEntity, In);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuCapacitorEntity, Reset);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Out);
}


//*****************************************************************************
VuRetVal VuCapacitorEntity::In(const VuParams &params)
{
	mCharge++;

	if ( mCharge == mCapacity )
		mpScriptComponent->getPlug("Out")->execute(params);

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuCapacitorEntity::Reset(const VuParams &params)
{
	mCharge = 0;

	return VuRetVal();
}


//*****************************************************************************
// VuFrequencyCapEntity
//*****************************************************************************
class VuFrequencyCapEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuFrequencyCapEntity();

private:
	VuRetVal			In(const VuParams &params);
	VuRetVal			Reset(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	int					mTimerType;
	float				mMinDelay;

	double				mLastTime;

	enum { GAME_TIME, REAL_TIME };
};

IMPLEMENT_RTTI(VuFrequencyCapEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuFrequencyCapEntity);


//*****************************************************************************
VuFrequencyCapEntity::VuFrequencyCapEntity():
	mTimerType(GAME_TIME),
	mMinDelay(1),
	mLastTime(0)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// properties
	static VuStaticIntEnumProperty::Choice sTimerChoices[] =
	{
		{ "Game Time", GAME_TIME },
		{ "Real Time", REAL_TIME },
		{ VUNULL }
	};
	addProperty(new VuStaticIntEnumProperty("Timer Type", mTimerType, sTimerChoices));
	addProperty(new VuFloatProperty("Min Delay", mMinDelay));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuFrequencyCapEntity, In);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuFrequencyCapEntity, Reset);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Out);
}


//*****************************************************************************
VuRetVal VuFrequencyCapEntity::In(const VuParams &params)
{
	double curTime = 0.0;
	switch ( mTimerType )
	{
	case GAME_TIME:
		curTime = VuTickManager::IF()->getGameTime();
		break;
	case REAL_TIME:
		curTime = VuSys::IF()->getTime();
		break;
	}

	if ( curTime >= mLastTime + mMinDelay )
	{
		mLastTime = curTime;
		mpScriptComponent->getPlug("Out")->execute(params);
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuFrequencyCapEntity::Reset(const VuParams &params)
{
	mLastTime = 0.0f;

	return VuRetVal();
}
