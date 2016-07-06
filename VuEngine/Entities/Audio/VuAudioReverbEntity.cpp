//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AudioReverb entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAudioProperty.h"
#include "VuEngine/HAL/Audio/VuAudioEvent.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


#if VU_DISABLE_AUDIO

class VuAudioReverbEntity : public VuEntity
{
	DECLARE_RTTI
};
IMPLEMENT_RTTI(VuAudioReverbEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioReverbEntity);

#else // VU_DISABLE_AUDIO

class VuAudioReverbEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioReverbEntity();

	virtual void		onPostLoad() { modified(); }
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// event handlers
	void				OnReverbSettingChanged(const VuParams &params);

	// scripting
	VuRetVal			Activate(const VuParams &params = VuParams());
	VuRetVal			Deactivate(const VuParams &params = VuParams());

	void				modified();
	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;
	Vu3dLayoutComponent	*mp3dLayoutComponent;

	// properties
	std::string			mReverbName;
	bool				mbInitiallyActive;
	float				mMinDistance;
	float				mMaxDistance;

	bool				mActive;
	FMOD::EventReverb	*mpReverb;
};


IMPLEMENT_RTTI(VuAudioReverbEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioReverbEntity);


//*****************************************************************************
VuAudioReverbEntity::VuAudioReverbEntity():
	mbInitiallyActive(true),
	mMinDistance(50.0f),
	mMaxDistance(100.0f),
	mActive(false),
	mpReverb(VUNULL)
{
	// event handlers
	REG_EVENT_HANDLER(VuAudioReverbEntity, OnReverbSettingChanged);

	// properties
	addProperty(new VuAudioReverbNameProperty("Reverb Name", mReverbName)) -> setWatcher(this, &VuAudioReverbEntity::modified);
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));
	addProperty(new VuFloatProperty("Min Distance", mMinDistance)) -> setWatcher(this, &VuAudioReverbEntity::modified);
	addProperty(new VuFloatProperty("Max Distance", mMaxDistance)) -> setWatcher(this, &VuAudioReverbEntity::modified);

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100, false));
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioReverbEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioReverbEntity, Deactivate);

	// only translation makes sense
	mpTransformComponent->setMask(VuTransformComponent::TRANS);
	mpTransformComponent->setWatcher(&VuAudioReverbEntity::modified);

	mp3dLayoutComponent->setDrawMethod(this, &VuAudioReverbEntity::drawLayout);

	// updates transform component
	modified();
}

//*****************************************************************************
void VuAudioReverbEntity::onGameInitialize()
{
	FMODCALL(VuAudio::IF()->eventSystem()->createReverb(&mpReverb));

	modified();

	if ( mbInitiallyActive )
		Activate();
	else
		Deactivate();
}

//*****************************************************************************
void VuAudioReverbEntity::onGameRelease()
{
	if ( mpReverb )
	{
		mpReverb->release();
		mpReverb = VUNULL;
	}
}

//*****************************************************************************
void VuAudioReverbEntity::OnReverbSettingChanged(const VuParams &params)
{
	if ( mpReverb )
	{
		mpReverb->release();
		mpReverb = VUNULL;
	}

	FMODCALL(VuAudio::IF()->eventSystem()->createReverb(&mpReverb));

	modified();

	if ( mActive )
		Activate();
}

//*****************************************************************************
VuRetVal VuAudioReverbEntity::Activate(const VuParams &params)
{
	mActive = true;

	if ( mpReverb )
		mpReverb->setActive(true);

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAudioReverbEntity::Deactivate(const VuParams &params)
{
	mActive = false;

	if ( mpReverb )
		mpReverb->setActive(false);

	return VuRetVal();
}

//*****************************************************************************
void VuAudioReverbEntity::modified()
{
	if ( mpReverb && mReverbName.length() )
	{
		FMOD_VECTOR pos = VuAudio::toFmodVector(mpTransformComponent->getWorldPosition());
		mpReverb->set3DAttributes(&pos, mMinDistance, mMaxDistance);

		FMOD_REVERB_PROPERTIES props = FMOD_PRESET_OFF;
		VuAudio::IF()->getReverbPreset(mReverbName, props);
		FMODCALL(mpReverb->setProperties(&props));
	}

	VuVector3 vExtents(mMaxDistance, mMaxDistance, mMaxDistance);
	mp3dLayoutComponent->setLocalBounds(VuAabb(-vExtents, vExtents));
}

//*****************************************************************************
void VuAudioReverbEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix matMVP = mpTransformComponent->getWorldTransform()*params.mCamera.getViewProjMatrix();

		VuGfxUtil::IF()->drawSphereLines(VuColor(255,64,64), mMinDistance, 16, 16, matMVP);
		VuGfxUtil::IF()->drawSphereLines(VuColor(64,255,64), mMaxDistance, 16, 16, matMVP);
	}
}

#endif // VU_DISABLE_AUDIO
