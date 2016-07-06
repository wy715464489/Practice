//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Element virtual base class.
// 
//*****************************************************************************

#pragma once

class VuJsonContainer;
class VuVector2;
class VuRect;


class VuUIPageLayoutElement
{
public:
	virtual ~VuUIPageLayoutElement() {}

	virtual float	measureHeight(float width, const VuVector2 &scale) = 0;
	virtual void	draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale) = 0;
};
