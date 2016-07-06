//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuVector2.h"

class VuUIPageLayoutElement;
class VuJsonContainer;
class VuRect;


class VuUIPageLayout
{
public:
	VuUIPageLayout();
	~VuUIPageLayout();

	void		setLayout(const VuJsonContainer &pageLayout);
	void		clearLayout();

	float		measureHeight(float width, const VuVector2 &scale);
	void		draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale);

private:
	typedef VuArray<VuUIPageLayoutElement *> Elements;

	Elements	mElements;
};
