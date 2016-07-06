//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dLayoutComponent class
// 
//*****************************************************************************

#include "Vu3dLayoutComponent.h"


IMPLEMENT_RTTI(Vu3dLayoutComponent, VuComponent);


//*****************************************************************************
Vu3dLayoutComponent::Vu3dLayoutComponent(VuEntity *pOwner) :
	VuComponent(pOwner),
	mpDrawMethod(VUNULL),
	mpCollideMethod(VUNULL),
	mBounds(VuVector3(-1, -1, -1), VuVector3(1, 1, 1)),
	mForceVisible(false)
{
}

//*****************************************************************************
Vu3dLayoutComponent::~Vu3dLayoutComponent()
{
	delete mpDrawMethod;
	delete mpCollideMethod;
}

//*****************************************************************************
void Vu3dLayoutComponent::draw(const Vu3dLayoutDrawParams &params) const
{
	if ( mpDrawMethod )
		mpDrawMethod->execute(params);
}

//*****************************************************************************
bool Vu3dLayoutComponent::collideRay(const VuVector3 &v0, VuVector3 &v) const
{
	if ( mpCollideMethod )
		return mpCollideMethod->execute(v0, v);

	return false;
}

//*****************************************************************************
const VuAabb &Vu3dLayoutComponent::getLocalBounds() const
{
	if ( !mBounds.isValid() )
		return VuAabb::one();

	return mBounds;
}

//*****************************************************************************
Vu3dLayoutDrawParams::Vu3dLayoutDrawParams(const VuCamera &camera):
	mCamera(camera),
	mbSelected(false),
	mbDrawCollision(false),
	mbDrawFluids(false),
	mbForceHighLOD(false)
{
}