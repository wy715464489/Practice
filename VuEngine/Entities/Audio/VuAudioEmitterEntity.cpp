//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AudioEmitter entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAudioProperty.h"
#include "VuEngine/HAL/Audio/VuAudioEvent.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


class VuAudioEmitterEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuAudioEmitterEntity();

	virtual void		onGameInitialize();

private:
	// scripting
	VuRetVal			Start(const VuParams &params = VuParams());
	VuRetVal			Stop(const VuParams &params);

	void				modified();
	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	VuScriptComponent *		mpScriptComponent;
	Vu3dLayoutComponent *	mp3dLayoutComponent;
	VuMotionComponent *		mpMotionComponent;

	// properties
	std::string			mEventName;
	bool				mbInitiallyActive;

	VuAudioEvent		mEvent;
};


IMPLEMENT_RTTI(VuAudioEmitterEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioEmitterEntity);


//*****************************************************************************
VuAudioEmitterEntity::VuAudioEmitterEntity():
	mbInitiallyActive(false)
{
	// properties
	addProperty(new VuAudioEventNameProperty("Event Name", mEventName))	-> setWatcher(this, &VuAudioEmitterEntity::modified);
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100, false));
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioEmitterEntity, Start);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioEmitterEntity, Stop);

	// only translation makes sense
	mpTransformComponent->setMask(VuTransformComponent::TRANS);

	mp3dLayoutComponent->setDrawMethod(this, &VuAudioEmitterEntity::drawLayout);
}

//*****************************************************************************
void VuAudioEmitterEntity::onGameInitialize()
{
	if ( mbInitiallyActive )
		Start();
}

//*****************************************************************************
VuRetVal VuAudioEmitterEntity::Start(const VuParams &params)
{
	if ( mEvent.create(mEventName.c_str()) )
	{
		mEvent.set3DAttributes(&mpTransformComponent->getWorldPosition(), VUNULL, VUNULL);
		mEvent.start();
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAudioEmitterEntity::Stop(const VuParams &params)
{
	mEvent.release();

	return VuRetVal();
}

//*****************************************************************************
void VuAudioEmitterEntity::modified()
{
	if ( mEvent.active() )
		Start();
}

//*****************************************************************************
void VuAudioEmitterEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		#if !VU_DISABLE_AUDIO
		FMOD::Event	*pEvent;
		if ( VuAudio::IF()->eventSystem()->getEvent(mEventName.c_str(), FMOD_EVENT_INFOONLY, &pEvent) == FMOD_OK )
		{
			float minDist = 0.0f, maxDist = 0.0f;
			if ( VuAudio::IF()->getMinMaxDist(pEvent, minDist, maxDist) )
			{
				// draw sphere(s)
				VuMatrix matMVP = mpTransformComponent->getWorldTransform()*params.mCamera.getViewProjMatrix();

				if ( minDist > 0 )
					VuGfxUtil::IF()->drawSphereLines(VuColor(192,64,64), minDist, 8, 8, matMVP);

				if ( maxDist > minDist )
					VuGfxUtil::IF()->drawSphereLines(VuColor(64,192,64), maxDist, 8, 8, matMVP);
			}
		}
		#endif
	}
}

//*****************************************************************************
void VuAudioEmitterEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());

	if ( mEvent.active() )
	{
		mEvent.set3DAttributes(&mpMotionComponent->getWorldTransform().getTrans(), &mpMotionComponent->getWorldLinearVelocity(), VUNULL);
	}
}