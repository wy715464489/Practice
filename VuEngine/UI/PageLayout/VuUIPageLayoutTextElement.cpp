//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Page Layout Text Element.
// 
//*****************************************************************************

#include "VuUIPageLayoutTextElement.h"
#include "VuEngine/DB/VuStringDB.h"
#include "VuEngine/DB/VuFontDB.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuUIPageLayoutTextElement::VuUIPageLayoutTextElement(const VuJsonContainer &data):
	mFlags(VUGFX_TEXT_DRAW_WORDBREAK)
{
	mFont = data["Font"].asString();
	mStringID = data["StringID"].asString();

	const std::string &alignment = data["Align"].asString();
	if ( alignment == "Right" )
		mFlags |= VUGFX_TEXT_DRAW_RIGHT;
	else if ( alignment == "Center" )
		mFlags |= VUGFX_TEXT_DRAW_HCENTER;
	else if ( alignment == "RightEaLeft" )
	{
		if ( !(VuStringDB::IF() && VuStringDB::IF()->isCurrentLanguageEastAsian()) )
			mFlags |= VUGFX_TEXT_DRAW_RIGHT;
	}
}

//*****************************************************************************
float VuUIPageLayoutTextElement::measureHeight(float width, const VuVector2 &scale)
{
	const VuFontDB::VuEntry &fontEntry = VuFontDB::IF()->getFont(mFont);
	const std::string &strText = VuStringDB::IF()->getString(mStringID).c_str();
	VuVector2 size = VuFontDraw::measureString(fontEntry.font(), strText.c_str(), fontEntry.params(), width, mFlags, scale.mY/scale.mX);

	return size.mY;
}

//*****************************************************************************
void VuUIPageLayoutTextElement::draw(float depth, const VuRect &rect, float offsetY, float alpha, const VuVector2 &scale)
{
	const VuFontDB::VuEntry &fontEntry = VuFontDB::IF()->getFont(mFont);
	const std::string &strText = VuStringDB::IF()->getString(mStringID).c_str();
	VuFontDrawParams params = fontEntry.params();
	params.mClip = true;
	params.mClipRect = rect;
	VuGfxUtil::IF()->fontDraw()->drawString(depth, fontEntry.font(), strText.c_str(), params, rect, mFlags, alpha, VuVector2(0.0f, offsetY));
}
