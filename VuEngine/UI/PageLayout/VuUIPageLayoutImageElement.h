//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Image Element.
// 
//*****************************************************************************

#pragma once

#include "VuUIPageLayoutElement.h"
#include "VuEngine/Math/VuVector2.h"

class VuTextureAsset;


class VuUIPageLayoutImageElement : public VuUIPageLayoutElement
{
public:
	VuUIPageLayoutImageElement(const VuJsonContainer &data);
	~VuUIPageLayoutImageElement();

	virtual float	measureHeight(float width, const VuVector2 &scale);
	virtual void	draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale);

private:
	VuTextureAsset	*mpTextureAsset;
	std::string		mAlignment;
};
