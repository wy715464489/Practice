//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Prop entity
// 
//*****************************************************************************

#include "VuEngine/Entities/Objects/VuPropEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawStaticModelComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Components/Attach/VuOffsetAttachComponent.h"
#include "VuEngine/Components/RigidBody/VuRigidBodyComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"

IMPLEMENT_RTTI(VuPropEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPropEntity);


//*****************************************************************************
VuPropEntity::VuPropEntity():
	mbInitiallyVisible(true),
	mbVisible(false)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));
	addComponent(mpAttachComponent = new VuOffsetAttachComponent(this));
	addComponent(mp3dDrawStaticModelComponent = new Vu3dDrawStaticModelComponent(this));
	addComponent(mpRigidBodyComponent = new VuRigidBodyComponent(this));

	mp3dLayoutComponent->setDrawMethod(this, &VuPropEntity::drawLayout);
	mp3dLayoutComponent->setCollideMethod(this, &VuPropEntity::collideLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuPropEntity::transformModified);

	// properties
	addProperty(new VuBoolProperty("Initially Visible", mbInitiallyVisible));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPropEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPropEntity, Hide);
}

//*****************************************************************************
void VuPropEntity::onGameInitialize()
{
	mpRigidBodyComponent->setCollisionGroup(COL_ENGINE_STATIC_PROP);
	mpRigidBodyComponent->setCollisionMask(COL_EVERYTHING ^ COL_ENGINE_STATIC_PROP);
	mpRigidBodyComponent->createRigidBody();

	if ( mbInitiallyVisible )
		show();
}

//*****************************************************************************
void VuPropEntity::onGameRelease()
{
	hide();

	mpRigidBodyComponent->destroyRigidBody();
}

//*****************************************************************************
void VuPropEntity::show()
{
	mp3dDrawStaticModelComponent->show();
	mpRigidBodyComponent->addToWorld();
}

//*****************************************************************************
void VuPropEntity::hide()
{
	mp3dDrawStaticModelComponent->hide();
	mpRigidBodyComponent->removeFromWorld();
}

//*****************************************************************************
void VuPropEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbDrawCollision )
	{
		mpRigidBodyComponent->draw(VuColor(255,255,255), params.mCamera);
	}
	else
	{
		mp3dDrawStaticModelComponent->drawLayout(params);
	}
}

//*****************************************************************************
bool VuPropEntity::collideLayout(const VuVector3 &v0, VuVector3 &v1)
{
	return mp3dDrawStaticModelComponent->collideLayout(v0, v1);
}

//*****************************************************************************
void VuPropEntity::transformModified()
{
	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(mpTransformComponent->getWorldScale());
	mp3dDrawStaticModelComponent->updateVisibility(mat);

	mpRigidBodyComponent->transformModified(mpTransformComponent->getWorldTransform());
	mpRigidBodyComponent->scaleModified(mpTransformComponent->getWorldScale());

	mpAttachComponent->update(mpTransformComponent->getWorldTransform());
}

//*****************************************************************************
void VuPropEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform(), false);

	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(mpTransformComponent->getWorldScale());
	mp3dDrawStaticModelComponent->updateVisibility(mat);

	mpRigidBodyComponent->onMotionUpdate(mpMotionComponent);

	mpAttachComponent->update(mpMotionComponent->getWorldTransform(), mpMotionComponent->getWorldLinearVelocity(), mpMotionComponent->getWorldAngularVelocity());
}

//*****************************************************************************
void VuPropEntity::onMotionActivate()
{
	mpRigidBodyComponent->setCollisionFlags(mpRigidBodyComponent->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
}

//*****************************************************************************
void VuPropEntity::onMotionDeactivate()
{
	mpRigidBodyComponent->setCollisionFlags(mpRigidBodyComponent->getCollisionFlags() & (~btCollisionObject::CF_KINEMATIC_OBJECT));
}
