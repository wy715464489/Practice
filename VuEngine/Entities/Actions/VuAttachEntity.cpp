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
#include "VuEngine/Components/Attach/VuAttachComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"
#include "VuEngine/Properties/VuRotation3dProperty.h"


class VuAttachEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAttachEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	VuRetVal			Attach(const VuParams &params = VuParams());
	VuRetVal			Detach(const VuParams &params = VuParams());

	bool				getComponents(VuAttachComponent *&pAttachComponent, VuMotionComponent *&pMotionComponent);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mbAttachAtStart;
	VuVector3			mRelativePosition;
	VuVector3			mRelativeRotation;
	std::string			mNodeName;

	// references
	VuScriptRef			*mpParentEntityRef;
	VuScriptRef			*mpChildEntityRef;
};


IMPLEMENT_RTTI(VuAttachEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAttachEntity);


//*****************************************************************************
VuAttachEntity::VuAttachEntity():
	mbAttachAtStart(false),
	mRelativePosition(0,0,0),
	mRelativeRotation(0,0,0)
{
	// properties
	addProperty(new VuBoolProperty("Attach at Start", mbAttachAtStart));
	addProperty(new VuVector3Property("Relative Position", mRelativePosition));
	addProperty(new VuRotation3dProperty("Relative Rotation", mRelativeRotation));
	addProperty(new VuStringProperty("Node Name", mNodeName));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAttachEntity, Attach);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAttachEntity, Detach);
	mpParentEntityRef = ADD_SCRIPT_REF(mpScriptComponent, Parent, VuEntity::msRTTI);
	mpChildEntityRef = ADD_SCRIPT_REF(mpScriptComponent, Child, VuEntity::msRTTI);
}

//*****************************************************************************
void VuAttachEntity::onGameInitialize()
{
	if ( mbAttachAtStart )
		Attach();
}

//*****************************************************************************
void VuAttachEntity::onGameRelease()
{
	Detach();
}

//*****************************************************************************
VuRetVal VuAttachEntity::Attach(const VuParams &params)
{
	VuAttachComponent *pAttachComponent;
	VuMotionComponent *pMotionComponent;
	if ( getComponents(pAttachComponent, pMotionComponent) )
	{
		VuMatrix transform;
		transform.setEulerAngles(mRelativeRotation);
		transform.setTrans(mRelativePosition);
		pAttachComponent->attach(pMotionComponent, transform, mNodeName.c_str());

		// set initial transform from transform component
		if ( VuTransformComponent *pTransformComponent = pAttachComponent->getOwnerEntity()->getTransformComponent() )
			pAttachComponent->update(pTransformComponent->getWorldTransform());
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAttachEntity::Detach(const VuParams &params)
{
	VuAttachComponent *pAttachComponent;
	VuMotionComponent *pMotionComponent;
	if ( getComponents(pAttachComponent, pMotionComponent) )
	{
		pAttachComponent->detach(pMotionComponent);
	}
	return VuRetVal();
}

//*****************************************************************************
bool VuAttachEntity::getComponents(VuAttachComponent *&pAttachComponent, VuMotionComponent *&pMotionComponent)
{
	if ( mpParentEntityRef->getRefScript() && mpChildEntityRef->getRefScript() )
	{
		pAttachComponent = mpParentEntityRef->getRefEntity()->getComponent<VuAttachComponent>();
		pMotionComponent = mpChildEntityRef->getRefEntity()->getComponent<VuMotionComponent>();
		return pAttachComponent && pMotionComponent;
	}

	return false;
}
