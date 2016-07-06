//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Corona Occluder entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/RigidBody/VuRigidBodyComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"


class VuCoronaOccluderEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuCoronaOccluderEntity();

	virtual void			onPostLoad() { transformModified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

protected:
	void					drawLayout(const Vu3dLayoutDrawParams &params);
	void					transformModified();

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuRigidBodyComponent	*mpRigidBodyComponent;
};

IMPLEMENT_RTTI(VuCoronaOccluderEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuCoronaOccluderEntity);


//*****************************************************************************
VuCoronaOccluderEntity::VuCoronaOccluderEntity()
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpRigidBodyComponent = new VuRigidBodyComponent(this));

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuCoronaOccluderEntity::transformModified);

	mp3dLayoutComponent->setDrawMethod(this, &VuCoronaOccluderEntity::drawLayout);
}

//*****************************************************************************
void VuCoronaOccluderEntity::onGameInitialize()
{
	mpRigidBodyComponent->setCollisionGroup(COL_ENGINE_CORONA_OCCLUDER);
	mpRigidBodyComponent->setCollisionMask(COL_NOTHING);
	mpRigidBodyComponent->createRigidBody();
	mpRigidBodyComponent->addToWorld();
}

//*****************************************************************************
void VuCoronaOccluderEntity::onGameRelease()
{
	mpRigidBodyComponent->removeFromWorld();
	mpRigidBodyComponent->destroyRigidBody();
}
//*****************************************************************************
void VuCoronaOccluderEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbDrawCollision )
	{
		mpRigidBodyComponent->draw(VuColor(64,192,64), params.mCamera);
	}
}

//*****************************************************************************
void VuCoronaOccluderEntity::transformModified()
{
	mpRigidBodyComponent->transformModified(mpTransformComponent->getWorldTransform());
	mpRigidBodyComponent->scaleModified(mpTransformComponent->getWorldScale());
}
