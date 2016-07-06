//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Zone entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuBitFieldProperty.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


//*****************************************************************************
// VuZoneEntity
//*****************************************************************************

class VuZoneEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuZoneEntity();

	virtual void	onGameInitialize();

protected:
	void			applyBitsRecursive(VuEntity *pEntity, VUUINT32 zoneBits);

	// properties
	int				mZone;
};

IMPLEMENT_RTTI(VuZoneEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuZoneEntity);

//*****************************************************************************
VuZoneEntity::VuZoneEntity() : VuEntity(CAN_HAVE_CHILDREN),
	mZone(1)
{
	// properties
	addProperty(new VuIntProperty("Zone", mZone));
}

//*****************************************************************************
void VuZoneEntity::onGameInitialize()
{
	applyBitsRecursive(this, 1<<mZone);
}

//*****************************************************************************
void VuZoneEntity::applyBitsRecursive(VuEntity *pEntity, VUUINT32 zoneBits)
{
	for ( int i = 0; i < pEntity->getChildEntityCount(); i++ )
	{
		VuEntity *pChild = pEntity->getChildEntity(i);

		if ( Vu3dDrawComponent *p3dDrawComponent = pChild->getComponent<Vu3dDrawComponent>() )
			p3dDrawComponent->setZoneBits(zoneBits);

		applyBitsRecursive(pChild, zoneBits);
	}
}


//*****************************************************************************
// VuZoneMaskEntity
//*****************************************************************************

class VuZoneMaskEntity : public VuEntity, Vu3dDrawManager::VuZoneMaskIF
{
	DECLARE_RTTI

public:
	VuZoneMaskEntity();

	virtual void	onGameInitialize();
	virtual void	onGameRelease();

protected:
	virtual void	drawLayout(const Vu3dLayoutDrawParams &params) = 0;

	// components
	Vu3dLayoutComponent	*mp3dLayoutComponent;

	// properties
	VUUINT32			mZoneMask;
};

IMPLEMENT_RTTI(VuZoneMaskEntity, VuEntity);

//*****************************************************************************
VuZoneMaskEntity::VuZoneMaskEntity():
	mZoneMask(1)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));

	// properties
	addProperty(new VuBitFieldProperty("1", mZoneMask, 1<<1));
	addProperty(new VuBitFieldProperty("2", mZoneMask, 1<<2));
	addProperty(new VuBitFieldProperty("3", mZoneMask, 1<<3));
	addProperty(new VuBitFieldProperty("4", mZoneMask, 1<<4));
	addProperty(new VuBitFieldProperty("5", mZoneMask, 1<<5));
	addProperty(new VuBitFieldProperty("6", mZoneMask, 1<<6));
	addProperty(new VuBitFieldProperty("7", mZoneMask, 1<<7));
	addProperty(new VuBitFieldProperty("8", mZoneMask, 1<<8));
	addProperty(new VuBitFieldProperty("9", mZoneMask, 1<<9));
	addProperty(new VuBitFieldProperty("10", mZoneMask, 1<<10));
	addProperty(new VuBitFieldProperty("11", mZoneMask, 1<<11));
	addProperty(new VuBitFieldProperty("12", mZoneMask, 1<<12));
	addProperty(new VuBitFieldProperty("13", mZoneMask, 1<<13));
	addProperty(new VuBitFieldProperty("14", mZoneMask, 1<<14));
	addProperty(new VuBitFieldProperty("15", mZoneMask, 1<<15));
}

//*****************************************************************************
void VuZoneMaskEntity::onGameInitialize()
{
	Vu3dDrawManager::IF()->addZoneMask(this);
}

//*****************************************************************************
void VuZoneMaskEntity::onGameRelease()
{
	Vu3dDrawManager::IF()->removeZoneMask(this);
}


//*****************************************************************************
// VuZoneMaskBoxEntity
//*****************************************************************************

class VuZoneMaskBoxEntity : public VuZoneMaskEntity
{
	DECLARE_RTTI

protected:
	virtual VUUINT32	calcMask(const VuVector3 &pos) const;
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);
};

IMPLEMENT_RTTI(VuZoneMaskBoxEntity, VuZoneMaskEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuZoneMaskBoxEntity);


//*****************************************************************************
void VuZoneMaskBoxEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.scaleLocal(mpTransformComponent->getWorldScale());

		VuGfxUtil::IF()->drawAabbSolid(VuColor(128,255,128,128), mp3dLayoutComponent->getLocalBounds(), mat, params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
VUUINT32 VuZoneMaskBoxEntity::calcMask(const VuVector3 &pos) const
{
	const VuMatrix &transform = mpTransformComponent->getWorldTransform();
	const VuVector3 &extents = mpTransformComponent->getWorldScale();

	VuVector3 delta = pos - transform.getTrans();
	float distX = VuAbs(VuDot(transform.getAxisX(), delta));
	float distY = VuAbs(VuDot(transform.getAxisY(), delta));
	float distZ = VuAbs(VuDot(transform.getAxisZ(), delta));

	if ( (distX < extents.mX) && (distY < extents.mY) && (distZ < extents.mZ) )
		return mZoneMask;

	return 0;
}
