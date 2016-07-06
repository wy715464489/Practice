//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  2dLayoutComponent class
// 
//*****************************************************************************

#include "Vu2dLayoutComponent.h"


IMPLEMENT_RTTI(Vu2dLayoutComponent, VuComponent);


//*****************************************************************************
Vu2dLayoutComponent::~Vu2dLayoutComponent()
{
	delete mpDrawMethod;
}

//*****************************************************************************
void Vu2dLayoutComponent::draw(bool bSelected) const
{
	mpDrawMethod->execute(bSelected);
}
