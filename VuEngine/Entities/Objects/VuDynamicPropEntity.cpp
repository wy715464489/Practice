//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Prop entity
// 
//*****************************************************************************

#include "VuEngine/Entities/Objects/VuDynamicPropEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawStaticModelComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Attach/VuOffsetAttachComponent.h"
#include "VuEngine/Components/RigidBody/VuRigidBodyComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuTickManager.h"

IMPLEMENT_RTTI(VuDynamicPropEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDynamicPropEntity);


//*****************************************************************************
VuDynamicPropEntity::VuDynamicPropEntity():
	mbInitiallyVisible(true),
	mMass(100.0f),
	mCenterOfMass(0,0,0),
	mCollideWithStaticProps(true),
	mbVisible(false)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
	addComponent(mpAttachComponent = new VuOffsetAttachComponent(this));
	addComponent(mp3dDrawStaticModelComponent = new Vu3dDrawStaticModelComponent(this));
	addComponent(mpRigidBodyComponent = new VuRigidBodyComponent(this));

	mp3dLayoutComponent->setDrawMethod(this, &VuDynamicPropEntity::drawLayout);
	mp3dLayoutComponent->setCollideMethod(this, &VuDynamicPropEntity::collideLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuDynamicPropEntity::transformModified);

	// properties
	addProperty(new VuBoolProperty("Initially Visible", mbInitiallyVisible));
	addProperty(new VuFloatProperty("Mass", mMass)) -> setWatcher(this, &VuDynamicPropEntity::massModified);
	addProperty(new VuVector3Property("Center of Mass", mCenterOfMass)) -> setWatcher(this, &VuDynamicPropEntity::massModified);
	addProperty(new VuBoolProperty("Collide With Static Props", mCollideWithStaticProps));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDynamicPropEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDynamicPropEntity, Hide);
}

//*****************************************************************************
void VuDynamicPropEntity::onPostLoad()
{
	transformModified();
	massModified();
}

//*****************************************************************************
void VuDynamicPropEntity::onGameInitialize()
{
	mpRigidBodyComponent->setMass(mMass);
	mpRigidBodyComponent->setMotionState(this);
	mpRigidBodyComponent->setCollisionGroup(COL_ENGINE_DYNAMIC_PROP);
	mpRigidBodyComponent->setCollisionMask(mCollideWithStaticProps ? COL_EVERYTHING : ~COL_ENGINE_STATIC_PROP);
	mpRigidBodyComponent->createRigidBody();

	VUASSERT(mpRigidBodyComponent->getRigidBody(), "Dynamic Prop without collision!");

	if ( mbInitiallyVisible )
		show();

	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &VuDynamicPropEntity::tickBuild, "Build");
}

//*****************************************************************************
void VuDynamicPropEntity::onGameRelease()
{
	// unregister phased ticks
	VuTickManager::IF()->unregisterHandlers(this);

	hide();

	mpRigidBodyComponent->destroyRigidBody();
}

//*****************************************************************************
void VuDynamicPropEntity::getWorldTransform(btTransform& worldTrans) const
{
	VuMatrix matRB = mpTransformComponent->getWorldTransform();
	matRB.translateLocal(mpRigidBodyComponent->getCenterOfMass());

	worldTrans = VuDynamicsUtil::toBtTransform(matRB);
}

//*****************************************************************************
void VuDynamicPropEntity::setWorldTransform(const btTransform& worldTrans)
{
	VuMatrix matModel = VuDynamicsUtil::toVuMatrix(worldTrans);
	matModel.translateLocal(-mpRigidBodyComponent->getCenterOfMass());

	mpTransformComponent->setWorldTransform(matModel, false);

	mpAttachComponent->update(mpTransformComponent->getWorldTransform());
}

//*****************************************************************************
void VuDynamicPropEntity::show()
{
	if ( !mbVisible )
	{
		mbVisible = true;

		mp3dDrawStaticModelComponent->show();

		mpRigidBodyComponent->addToWorld();
	}
}

//*****************************************************************************
void VuDynamicPropEntity::hide()
{
	if ( mbVisible )
	{
		mbVisible = false;

		mp3dDrawStaticModelComponent->hide();

		mpRigidBodyComponent->removeFromWorld();
	}
}

//*****************************************************************************
void VuDynamicPropEntity::drawLayout(const Vu3dLayoutDrawParams &params)
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
bool VuDynamicPropEntity::collideLayout(const VuVector3 &v0, VuVector3 &v1)
{
	return mp3dDrawStaticModelComponent->collideLayout(v0, v1);
}

//*****************************************************************************
void VuDynamicPropEntity::transformModified()
{
	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(mpTransformComponent->getWorldScale());
	mp3dDrawStaticModelComponent->updateVisibility(mat);

	mpRigidBodyComponent->transformModified(mpTransformComponent->getWorldTransform());
	mpRigidBodyComponent->scaleModified(mpTransformComponent->getWorldScale());

	mpAttachComponent->update(mpTransformComponent->getWorldTransform());
}

//*****************************************************************************
void VuDynamicPropEntity::massModified()
{
	mpRigidBodyComponent->setMass(mMass);
	mpRigidBodyComponent->setCenterOfMass(mCenterOfMass);
}

//*****************************************************************************
void VuDynamicPropEntity::tickBuild(float fdt)
{
	// update visibility
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.scaleLocal(mpTransformComponent->getWorldScale());
		mp3dDrawStaticModelComponent->updateVisibility(mat);
	}
}