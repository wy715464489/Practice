//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Text Element.
// 
//*****************************************************************************

#pragma once

#include "VuUIPageLayoutElement.h"
#include "VuEngine/Math/VuVector2.h"


class VuUIPageLayoutTextElement : public VuUIPageLayoutElement
{
public:
	VuUIPageLayoutTextElement(const VuJsonContainer &data);

	virtual float	measureHeight(float width, const VuVector2 &scale);
	virtual void	draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale);

private:
	std::string		mFont;
	std::string		mStringID;
	int				mFlags;
};
