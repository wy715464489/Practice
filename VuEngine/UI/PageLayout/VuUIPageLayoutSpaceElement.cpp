//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Space Element.
// 
//*****************************************************************************

#include "VuUIPageLayoutSpaceElement.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuUIPageLayoutSpaceElement::VuUIPageLayoutSpaceElement(const VuJsonContainer &data)
{
	mAmount = data["Amount"].asFloat();
}

//*****************************************************************************
float VuUIPageLayoutSpaceElement::measureHeight(float width, const VuVector2 &scale)
{
	return mAmount*scale.mY;
}

//*****************************************************************************
void VuUIPageLayoutSpaceElement::draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale)
{
}
