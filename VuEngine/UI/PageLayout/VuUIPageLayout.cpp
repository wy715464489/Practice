//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout class.
// 
//*****************************************************************************

#include "VuUIPageLayout.h"
#include "VuUIPageLayoutTextElement.h"
#include "VuUIPageLayoutImageElement.h"
#include "VuUIPageLayoutSpaceElement.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Math/VuRect.h"


//*****************************************************************************
VuUIPageLayout::VuUIPageLayout()
{
}

//*****************************************************************************
VuUIPageLayout::~VuUIPageLayout()
{
	clearLayout();
}

//*****************************************************************************
void VuUIPageLayout::setLayout(const VuJsonContainer &pageLayout)
{
	clearLayout();

	const VuJsonContainer &elements = pageLayout["Elements"];
	for ( int i = 0; i < elements.size(); i++ )
	{
		const VuJsonContainer &element = elements[i];
		const std::string &type = element["Type"].asString();

		if ( type == "Text" )
		{
			mElements.push_back(new VuUIPageLayoutTextElement(element));
		}
		else if ( type == "Image" )
		{
			mElements.push_back(new VuUIPageLayoutImageElement(element));
		}
		else if ( type == "Space" )
		{
			mElements.push_back(new VuUIPageLayoutSpaceElement(element));
		}
	}
}

//*****************************************************************************
void VuUIPageLayout::clearLayout()
{
	for ( int i = 0; i < mElements.size(); i++ )
		delete mElements[i];
	mElements.clear();
}

//*****************************************************************************
float VuUIPageLayout::measureHeight(float width, const VuVector2 &scale)
{
	float totalHeight = 0;

	for ( int i = 0; i < mElements.size(); i++ )
		totalHeight += mElements[i]->measureHeight(width, scale);

	return totalHeight;
}

//*****************************************************************************
void VuUIPageLayout::draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale)
{
	float y = rect.mY + offsetY;

	for ( int i = 0; i < mElements.size() && y <= rect.getBottom(); i++ )
	{
		float height = mElements[i]->measureHeight(rect.mWidth, scale);

		if ( y + height >= rect.mY )
			mElements[i]->draw(depth, rect, y - rect.mY, alpha, scale);

		y += height;
	}
}
