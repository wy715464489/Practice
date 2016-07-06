//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DepthFogComponent class
// 
//*****************************************************************************

#include "VuDepthFogComponent.h"
#include "Vu3dDrawStaticModelComponent.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/Water/VuWaterSurfaceEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuEnumProperty.h"


IMPLEMENT_RTTI(VuDepthFogComponent, VuComponent);

#define DEFAULT_WATER_Z (-1e9)


//*****************************************************************************
VuDepthFogComponent::VuDepthFogComponent(VuEntity *pOwner):
	VuComponent(pOwner),
	mLocation(LOC_PIVOT),
	mManualWaterZ(0.0f)
{
	// properties
	static VuStaticIntEnumProperty::Choice sLocationChoices[] =
	{
		{ "Pivot", LOC_PIVOT},
		{ "Aabb", LOC_AABB},
		{ "Manual", LOC_MANUAL},
		{ VUNULL }
	};
	addProperty(new VuStaticIntEnumProperty("Location", mLocation, sLocationChoices));
	addProperty(new VuFloatProperty("Manual Water Z", mManualWaterZ));
}

//*****************************************************************************
void VuDepthFogComponent::onBake()
{
	if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		float waterZ = -1e9;

		if ( mLocation == LOC_MANUAL )
		{
			waterZ = mManualWaterZ;
		}
		else
		{
			VuVector3 pos = getOwnerEntity()->getTransformComponent()->getWorldPosition();
			if ( mLocation == LOC_AABB )
				pos = pDrawComponent->getAabb().getCenter();

			VuVector3 v0(pos.mX, pos.mY, 1e5);
			VuVector3 v1(pos.mX, pos.mY, -1e5);
			collideRay(getOwnerEntity()->getRootEntity(), v0, v1);

			waterZ = v1.mZ;
		}

		pDrawComponent->modelInstance().setWaterZ(waterZ);
		pDrawComponent->lod1ModelInstance().setWaterZ(waterZ);
		pDrawComponent->lod2ModelInstance().setWaterZ(waterZ);
		pDrawComponent->reflectionModelInstance().setWaterZ(waterZ);
	}
}

//*****************************************************************************
void VuDepthFogComponent::collideRay(VuEntity *pEntity, const VuVector3 &v0, VuVector3 &v1)
{
	if ( pEntity->isDerivedFrom(VuWaterSurfaceEntity::msRTTI) )
		if ( Vu3dLayoutComponent *p3dLayoutComponent = pEntity->getComponent<Vu3dLayoutComponent>() )
			p3dLayoutComponent->collideRay(v0, v1);

	// recurse
	for ( int iChild = 0; iChild < pEntity->getChildEntityCount(); iChild++ )
		collideRay(pEntity->getChildEntity(iChild), v0, v1);
}

//*****************************************************************************
void VuDepthFogComponent::loadDepthFog(const VuJsonContainer &data)
{
	if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		float waterZ = DEFAULT_WATER_Z;
		data["WaterZ"].getValue(waterZ);

		pDrawComponent->modelInstance().setWaterZ(waterZ);
		pDrawComponent->lod1ModelInstance().setWaterZ(waterZ);
		pDrawComponent->lod2ModelInstance().setWaterZ(waterZ);
		pDrawComponent->reflectionModelInstance().setWaterZ(waterZ);
		pDrawComponent->ultraModelInstance().setWaterZ(waterZ);
	}
}

//*****************************************************************************
void VuDepthFogComponent::saveDepthFog(VuJsonContainer &data) const
{
	if ( Vu3dDrawStaticModelComponent *pDrawComponent = getOwnerEntity()->getComponent<Vu3dDrawStaticModelComponent>() )
	{
		float waterZ = pDrawComponent->modelInstance().getWaterZ();
		if ( waterZ != DEFAULT_WATER_Z )
			data["WaterZ"].putValue(pDrawComponent->modelInstance().getWaterZ());
	}
}
