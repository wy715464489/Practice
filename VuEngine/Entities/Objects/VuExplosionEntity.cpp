//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Explosion entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/Managers/VuExplosionManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


class VuExplosionEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuExplosionEntity();

private:

	VuRetVal			Trigger(const VuParams &params);

	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// components
	Vu3dLayoutComponent	*mp3dLayoutComponent;
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mExplosionType;

	// property references
	VuDBEntryProperty	*mpExplosionTypeProperty;
};


IMPLEMENT_RTTI(VuExplosionEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuExplosionEntity);


//*****************************************************************************
VuExplosionEntity::VuExplosionEntity()
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	// properties
	addProperty(mpExplosionTypeProperty = new VuDBEntryProperty("Explosion Type", mExplosionType, "ExplosionDB"));

	mp3dLayoutComponent->setDrawMethod(this, &VuExplosionEntity::drawLayout);

	ADD_SCRIPT_INPUT(mpScriptComponent, VuExplosionEntity, Trigger, VuRetVal::Void, VuParamDecl());
}

//*****************************************************************************
VuRetVal VuExplosionEntity::Trigger(const VuParams &params)
{
	VuExplosionManager::IF()->createExplosion(mpTransformComponent->getWorldPosition(), mpExplosionTypeProperty->getEntryData(), this);

	return VuRetVal();
}

//*****************************************************************************
void VuExplosionEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		float outerRadius = mpExplosionTypeProperty->getEntryData()["Outer Radius"].asFloat();
		float innerRadius = mpExplosionTypeProperty->getEntryData()["Inner Radius"].asFloat();

		VuMatrix mat = mpTransformComponent->getWorldTransform()*params.mCamera.getViewProjMatrix();

		VuGfxUtil::IF()->drawSphereLines(VuColor(255, 128, 128), innerRadius, 8, 8, mat);

		if ( outerRadius > innerRadius )
			VuGfxUtil::IF()->drawSphereLines(VuColor(128, 255, 128), outerRadius, 8, 8, mat);
	}
}
