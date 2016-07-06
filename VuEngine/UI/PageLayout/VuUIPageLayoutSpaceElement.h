//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Space Element.
// 
//*****************************************************************************

#pragma once

#include "VuUIPageLayoutElement.h"


class VuUIPageLayoutSpaceElement : public VuUIPageLayoutElement
{
public:
	VuUIPageLayoutSpaceElement(const VuJsonContainer &data);

	virtual float	measureHeight(float width, const VuVector2 &scale);
	virtual void	draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale);

private:
	float			mAmount;
};
