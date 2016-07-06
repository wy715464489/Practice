//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Prop entity
// 
//*****************************************************************************

#include "VuAnimatedPropEntity.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawAnimatedModelComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Attach/VuAnimatedAttachComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Components/RigidBody/VuRigidBodyComponent.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAnimatedModelAsset.h"
#include "VuEngine/Assets/VuAnimationAsset.h"
#include "VuEngine/Assets/VuTimedEventAsset.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Animation/VuAnimatedSkeleton.h"
#include "VuEngine/Animation/VuAnimationControl.h"
#include "VuEngine/Animation/VuSkeleton.h"
#include "VuEngine/Animation/VuAnimation.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/VuPfxEntity.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevUtil.h"


IMPLEMENT_RTTI(VuAnimatedPropEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAnimatedPropEntity);


// static variables
static bool sbDebugAnimation = false;


//*****************************************************************************
VuAnimatedPropEntity::VuAnimatedPropEntity():
	mbInitiallyVisible(true),
	mCollisionGroup(COL_ENGINE_STATIC_PROP),
	mCollisionMask((VUUINT32)(COL_EVERYTHING ^ COL_ENGINE_STATIC_PROP)),
	mActive(false),
	mbBlending(false),
	mBlendRate(0.0f)
{
	// properties
	addProperty(new VuBoolProperty("Initially Visible", mbInitiallyVisible));

	// components
	addComponent(mp3dDrawAnimatedModelComponent = new Vu3dDrawAnimatedModelComponent(this));
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this));
	addComponent(mpAttachComponent = new VuAnimatedAttachComponent(this, &mp3dDrawAnimatedModelComponent->getModelInstance()));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));
	addComponent(mpRigidBodyComponent = new VuRigidBodyComponent(this));

	mp3dLayoutComponent->setDrawMethod(this, &VuAnimatedPropEntity::drawLayout);

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAnimatedPropEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAnimatedPropEntity, Hide);
	ADD_SCRIPT_INPUT(mpScriptComponent, VuAnimatedPropEntity, PlayAnimation, VuRetVal::Void, VuParamDecl(5, VuParams::Asset, VuParams::Float, VuParams::Float, VuParams::Float, VuParams::Bool));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuAnimatedPropEntity, AddAdditiveAnimation, VuRetVal::Void, VuParamDecl(5, VuParams::UnsignedInt, VuParams::Asset, VuParams::Float, VuParams::Float, VuParams::Bool));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuAnimatedPropEntity, RemoveAdditiveAnimation, VuRetVal::Void, VuParamDecl(2, VuParams::UnsignedInt, VuParams::Float));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuAnimatedPropEntity, SetAlpha, VuRetVal::Void, VuParamDecl(1, VuParams::Float));
	ADD_SCRIPT_INPUT(mpScriptComponent, VuAnimatedPropEntity, SetAdditiveAlpha, VuRetVal::Void, VuParamDecl(1, VuParams::Float));
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnAnimStart);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnAnimDone);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnAnimLoop);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuAnimatedPropEntity::transformModified);

	static VuDevBoolOnce sbOnce;
	if ( sbOnce && VuDevMenu::IF() )
	{
		VuDevMenu::IF()->addBool("Animation/Debug", sbDebugAnimation);
	}
}

//*****************************************************************************
VuAnimatedPropEntity::~VuAnimatedPropEntity()
{
}

//*****************************************************************************
void VuAnimatedPropEntity::onGameInitialize()
{
	mpRigidBodyComponent->setCollisionGroup((VUINT16)mCollisionGroup);
	mpRigidBodyComponent->setCollisionMask((VUINT16)mCollisionMask);
	mpRigidBodyComponent->createRigidBody();

	if ( mbInitiallyVisible )
		show();

	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &VuAnimatedPropEntity::tickAnim, "Anim");
}

//*****************************************************************************
void VuAnimatedPropEntity::onGameRelease()
{
	hide();

	for ( auto &entry : mAdditiveAnimations )
		entry.second.mpAnimControl->removeRef();
	mAdditiveAnimations.clear();

	mpRigidBodyComponent->destroyRigidBody();

	// unregister phased ticks
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
VuRetVal VuAnimatedPropEntity::PlayAnimation(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	VuAsset *pAsset = accessor.getAsset();
	float startTime = accessor.getFloat();
	float blendTime = accessor.getFloat();
	float timeFactor = accessor.getFloat();
	bool looping = accessor.getBool();
	VuAsset *pTimedEventAsset = VUNULL;
	if ( accessor.getNextType() == VuParams::Asset )
		pTimedEventAsset = accessor.getAsset();
	float skipTime = 0;
	if ( accessor.getNextType() == VuParams::Float )
		skipTime = accessor.getFloat();

	VUASSERT(pAsset->isDerivedFrom(VuAnimationAsset::msRTTI), "VuAnimatedPropEntity::PlayAnimation() expecting VuAnimationAsset");
	VuAnimationAsset *pAnimationAsset = static_cast<VuAnimationAsset *>(pAsset);

	if ( VuAnimatedSkeleton *pAnimatedSkeleton = mp3dDrawAnimatedModelComponent->getAnimatedSkeleton() )
	{
		VuAnimation *pAnimation = pAnimationAsset->getAnimation();
		VUASSERT(pAnimation->getBoneCount() == pAnimatedSkeleton->getSkeleton()->mBoneCount, "VuAnimatedPropEntity::PlayAnimation() bone count mismatch");

		VUASSERT(pAnimation->isAdditive() == false, "VuUiDriverSlotEntity::PlayAnimation() expected non-additive animation");
		if ( pAnimation->isAdditive() == false )
		{
			VuAnimationControl *pAnimationControl = new VuAnimationControl(pAnimation);
			pAnimationControl->setLocalTime(startTime);
			pAnimationControl->setTimeFactor(timeFactor);
			pAnimationControl->setLooping(looping);

			// start blending (if desired)
			if ( blendTime > 0 )
			{
				mbBlending = true;
				mBlendRate = 1.0f/blendTime;
				pAnimationControl->setWeight(0.0f);
			}
			else
			{
				pAnimatedSkeleton->clearBlendAnimationControls();
			}

			pAnimatedSkeleton->addAnimationControl(pAnimationControl);

			if ( pTimedEventAsset )
			{
				VUASSERT(pTimedEventAsset->isDerivedFrom(VuTimedEventAsset::msRTTI), "VuAnimatedPropEntity::PlayAnimation() expecting VuTimedEventAsset");
				pAnimationControl->setTimedEventAsset(static_cast<VuTimedEventAsset *>(pTimedEventAsset));
			}
			pAnimationControl->setEventIF(this);

			pAnimationControl->advance(skipTime);

			pAnimationControl->removeRef();
		}
	}

	mpScriptComponent->getPlug("OnAnimStart")->execute();

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAnimatedPropEntity::AddAdditiveAnimation(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	VUUINT32 id = accessor.getUnsignedInt();
	VuAsset *pAsset = accessor.getAsset();
	float blendInTime = accessor.getFloat();
	float timeFactor = accessor.getFloat();
	bool looping = accessor.getBool();

	VUASSERT(pAsset->isDerivedFrom(VuAnimationAsset::msRTTI), "VuAnimatedPropEntity::AddAdditiveAnimation() expecting VuAnimationAsset");
	VuAnimationAsset *pAnimationAsset = static_cast<VuAnimationAsset *>(pAsset);

	if ( VuAnimatedSkeleton *pAnimatedSkeleton = mp3dDrawAnimatedModelComponent->getAnimatedSkeleton() )
	{
		AdditiveAnimations::iterator iter = mAdditiveAnimations.find(id);
		if ( iter == mAdditiveAnimations.end() )
		{
			if ( pAnimatedSkeleton )
			{
				VuAnimation *pAnimation = pAnimationAsset->getAnimation();
				VUASSERT(pAnimation->getBoneCount() == pAnimatedSkeleton->getSkeleton()->mBoneCount, "VuAnimatedPropEntity::AddAdditiveAnimation() bone count mismatch");

				VUASSERT(pAnimation->isAdditive() == true, "VuAnimatedPropEntity::AddAdditiveAnimation() expected additive animation");
				if ( pAnimation->isAdditive() == true )
				{
					AdditiveAnimation &anim = mAdditiveAnimations[id];

					anim.mpAnimControl = new VuAnimationControl(pAnimation);

					anim.mpAnimControl->setTimeFactor(timeFactor);
					anim.mpAnimControl->setLooping(looping);

					if ( blendInTime > 0 )
					{
						anim.mBlendRate = 1.0f/blendInTime;
						anim.mpAnimControl->setWeight(0.0f);
					}

					pAnimatedSkeleton->addAnimationControl(anim.mpAnimControl);
				}
			}
		}
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAnimatedPropEntity::RemoveAdditiveAnimation(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	VUUINT32 id = accessor.getUnsignedInt();
	float blendOutTime = accessor.getFloat();

	if ( VuAnimatedSkeleton *pAnimatedSkeleton = mp3dDrawAnimatedModelComponent->getAnimatedSkeleton() )
	{
		AdditiveAnimations::iterator iter = mAdditiveAnimations.find(id);
		if ( iter != mAdditiveAnimations.end() )
		{
			if ( blendOutTime > 0 )
			{
				iter->second.mBlendRate = -1.0f/blendOutTime;
			}
			else
			{
				pAnimatedSkeleton->removeAnimationControl(iter->second.mpAnimControl);
				iter->second.mpAnimControl->removeRef();
				mAdditiveAnimations.erase(iter);
			}
		}
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAnimatedPropEntity::SetAlpha(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	float alpha = accessor.getFloat();

	mp3dDrawAnimatedModelComponent->setAlpha(alpha);

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAnimatedPropEntity::SetAdditiveAlpha(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	float alpha = accessor.getFloat();

	mp3dDrawAnimatedModelComponent->setAdditiveAlpha(alpha);

	return VuRetVal();
}

//*****************************************************************************
void VuAnimatedPropEntity::show()
{
	mp3dDrawAnimatedModelComponent->show();
	mpRigidBodyComponent->addToWorld();

	mActive = true;
}

//*****************************************************************************
void VuAnimatedPropEntity::hide()
{
	mp3dDrawAnimatedModelComponent->hide();
	mpRigidBodyComponent->removeFromWorld();

	mActive = false;
}

//*****************************************************************************
void VuAnimatedPropEntity::tickAnim(float fdt)
{
	if ( VuAnimatedSkeleton *pAnimatedSkeleton = mp3dDrawAnimatedModelComponent->getAnimatedSkeleton() )
	{
		// handle blending
		if ( mbBlending )
		{
			int controlCount = pAnimatedSkeleton->getBlendAnimationControlCount();
			float blendDelta = mBlendRate*fdt;
			for ( int i = 0; i < controlCount - 1; i++ )
			{
				VuAnimationControl *pAnimationControl = pAnimatedSkeleton->getBlendAnimationControl(i);
				float weight = pAnimationControl->getWeight();
				weight = VuMax(weight - blendDelta, 0.0f);
				pAnimationControl->setWeight(weight);
			}
			if ( controlCount > 0 )
			{
				VuAnimationControl *pAnimationControl = pAnimatedSkeleton->getBlendAnimationControl(controlCount - 1);
				float weight = pAnimationControl->getWeight();
				weight = VuMin(weight + blendDelta, 1.0f);
				pAnimationControl->setWeight(weight);
				if ( weight >= 1.0f )
				{
					mbBlending = false;
					mBlendRate = 0.0f;
				}
			}
		}

		// remove old anims that have reached a weight of 0
		for ( int i = pAnimatedSkeleton->getBlendAnimationControlCount() - 2; i >= 0; i-- )
			if ( pAnimatedSkeleton->getBlendAnimationControl(i)->getWeight() <= 0.0f )
				pAnimatedSkeleton->removeAnimationControl(pAnimatedSkeleton->getBlendAnimationControl(i));

		// additive animations
		for ( auto iter = mAdditiveAnimations.begin(); iter != mAdditiveAnimations.end(); )
		{
			float weight = iter->second.mpAnimControl->getWeight();
			weight = VuClamp(weight + iter->second.mBlendRate*fdt, 0.0f, 1.0f);
			iter->second.mpAnimControl->setWeight(weight);

			if (iter->second.mBlendRate < 0.0f && weight < FLT_EPSILON)
			{
				// Adjusted for compilation under NDK/Clang
				// Postincrement of the iterator in the parameter list to erase() guarantees
				// that the value of iter is pushed on the stack for the call to erase() and
				// then the increment to the iterator happens BEFORE the function call, so
				// the iterator will still be valid
				pAnimatedSkeleton->removeAnimationControl(iter->second.mpAnimControl);
				iter->second.mpAnimControl->removeRef();
				mAdditiveAnimations.erase(iter++);
			}
			else
			{
				++iter;
			}
		}

		pAnimatedSkeleton->advance(fdt);
		pAnimatedSkeleton->build();

		mp3dDrawAnimatedModelComponent->getModelInstance().setPose(pAnimatedSkeleton);
		mp3dDrawAnimatedModelComponent->getModelInstance().finalizePose();

		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.scaleLocal(mpTransformComponent->getWorldScale());
		mp3dDrawAnimatedModelComponent->updateVisibility(mat);

		if ( sbDebugAnimation )
		{
			pAnimatedSkeleton->drawInfo(mat.getTrans());
		}
	}

	mpAttachComponent->update(mpTransformComponent->getWorldTransform(), mpMotionComponent->getWorldLinearVelocity(), mpMotionComponent->getWorldAngularVelocity());
}

//*****************************************************************************
void VuAnimatedPropEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbDrawCollision )
	{
		mpRigidBodyComponent->draw(VuColor(255,255,255), params.mCamera);
	}
	else
	{
		mp3dDrawAnimatedModelComponent->drawLayout(params);
	}
}

//*****************************************************************************
void VuAnimatedPropEntity::transformModified()
{
	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(mpTransformComponent->getWorldScale());
	mp3dDrawAnimatedModelComponent->updateVisibility(mat);

	mpRigidBodyComponent->transformModified(mpTransformComponent->getWorldTransform());
	mpRigidBodyComponent->scaleModified(mpTransformComponent->getWorldScale());
}

//*****************************************************************************
void VuAnimatedPropEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform(), false);

	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(mpTransformComponent->getWorldScale());
	mp3dDrawAnimatedModelComponent->updateVisibility(mat);

	mpRigidBodyComponent->onMotionUpdate(mpMotionComponent);
}

//*****************************************************************************
void VuAnimatedPropEntity::onMotionActivate()
{
	mpRigidBodyComponent->setCollisionFlags(mpRigidBodyComponent->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
}

//*****************************************************************************
void VuAnimatedPropEntity::onMotionDeactivate()
{
	mpRigidBodyComponent->setCollisionFlags(mpRigidBodyComponent->getCollisionFlags() & (~btCollisionObject::CF_KINEMATIC_OBJECT));
}

//*****************************************************************************
void VuAnimatedPropEntity::onAnimationEvent(const std::string &type, const VuJsonContainer &params)
{
	if ( type == "AnimDone" )
	{
		mpScriptComponent->getPlug("OnAnimDone")->execute();
	}
	else if ( type == "AnimLoop" )
	{
		mpScriptComponent->getPlug("OnAnimLoop")->execute();
	}
	else if ( type == "PlayAudioEvent" )
	{
#if !VU_DISABLE_AUDIO
		FMOD::Event *pEvent;
		if ( VuAudio::IF()->eventSystem()->getEvent(params["EventName"].asCString(), FMOD_EVENT_NONBLOCKING, &pEvent) == FMOD_OK )
		{
			VuMatrix modelMat = mpTransformComponent->getWorldTransform();
			modelMat.scaleLocal(mpTransformComponent->getWorldScale());

			VuVector3 pos(0,0,0);
			VuDataUtil::getValue(params["Pos"], pos);
			pos = modelMat.transform(pos);

			FMOD_VECTOR fmodPos = VuAudio::toFmodVector(pos);
			pEvent->set3DAttributes(&fmodPos, VUNULL, VUNULL);

			pEvent->start();
		}
#endif
	}
	else if ( type == "PlayPfxModelSpace" )
	{
		// create effect
		if ( VUUINT32 hPfx = VuPfxManager::IF()->createEntity(params["PfxName"].asCString(), true) )
		{
			if ( VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(hPfx) )
			{
				VuMatrix modelMat = mpTransformComponent->getWorldTransform();
				modelMat.scaleLocal(mpTransformComponent->getWorldScale());

				VuVector3 pos(0,0,0);
				VuVector3 rot(0,0,0);
				VuDataUtil::getValue(params["Pos"], pos);
				VuDataUtil::getValue(params["Rot"], rot);
				VuMatrix mat;
				mat.setEulerAngles(rot);
				mat.setTrans(pos);
				mat = mat*modelMat;

				pPfxEntity->getSystemInstance()->setMatrix(mat);
				pPfxEntity->getSystemInstance()->start();
			}
		}
	}
	else if ( type == "Show" )
	{
		show();
	}
	else if ( type == "Hide" )
	{
		hide();
	}
	else if ( type == "BroadcastGenericEvent" )
	{
		const std::string &eventName = params["EventName"].asString();
		if ( eventName.length() )
		{
			VuParams outParams;
			outParams.addString(eventName.c_str());
			VuEventManager::IF()->broadcast("OnGenericEvent", outParams);
		}
	}
}
