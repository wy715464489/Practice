//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Text Base class
// 
//*****************************************************************************

#include "VuUITextBaseEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuFontEnumProperty.h"
#include "VuEngine/Assets/VuFontAsset.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"


IMPLEMENT_RTTI(VuUITextBaseEntity, VuEntity);


//*****************************************************************************
VuUITextBaseEntity::VuUITextBaseEntity():
	mbVisible(true),
	mRect(0,0,100,100),
	mOffset(0,0),
	mAlpha(1.0f)
{
	// properties
	addProperty(new VuBoolProperty("Visible", mbVisible));
	ADD_UI_RECT_PROPERTIES(getProperties(), mRect, "");
	addProperty(new VuFontEnumProperty("Font", mFont));
	ADD_UI_STRING_FORMAT_PROPERTIES(getProperties(), mStringFormat, "");
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
	addComponent(new Vu2dLayoutComponent(this, &VuUITextBaseEntity::drawLayout));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUITextBaseEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuUITextBaseEntity, Hide);
	ADD_SCRIPT_INPUT(mpScriptComponent, VuUITextBaseEntity, SetAlpha, VuRetVal::Void, VuParamDecl(1, VuParams::Float));

	// event handlers
	REG_EVENT_HANDLER(VuUITextBaseEntity, OnUIDraw);
}

//*****************************************************************************
void VuUITextBaseEntity::OnUIDraw(const VuParams &params)
{
	float alpha = 1.0f;

	if ( mbVisible )
	{
		drawText(getText(), alpha);
	}
}

//*****************************************************************************
VuRetVal VuUITextBaseEntity::SetAlpha(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	mAlpha = accessor.getFloat();

	return VuRetVal();
}

//*****************************************************************************
void VuUITextBaseEntity::drawLayout(bool bSelected)
{
	// draw rect
	if ( bSelected )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);

		const VuFontDB::VuEntry &fontEntry = VuFontDB::IF()->getFont(mFont);
		VuGfxUtil::IF()->drawRectangleOutline2d(uiDrawParams.mDepth, fontEntry.params().mColor, uiDrawParams.transform(mRect));
	}

	// draw text
	drawText(getText(), 1.0f);
}

//*****************************************************************************
VuVector2 VuUITextBaseEntity::measureString()
{
	const char *strText = getText();
	if ( strText && strText[0] )
	{
		const VuFontDB::VuEntry &fontEntry = VuFontDB::IF()->getFont(mFont);
		return VuFontDraw::measureString(fontEntry.font(), getText(), fontEntry.params(), mRect.mWidth, mStringFormat, VuUI::IF()->getAuthoringAspectRatio());
	}

	return VuVector2(0,0);
}

//*****************************************************************************
void VuUITextBaseEntity::drawText(const char *text, float alpha)
{
	if ( text && text[0] )
	{
		VuUIDrawParams uiDrawParams;
		VuUIDrawUtil::getParams(this, uiDrawParams);

		const VuFontDB::VuEntry &fontEntry = VuFontDB::IF()->getFont(mFont);

		VuRect rect = uiDrawParams.transform(mRect);
		mAnchor.apply(rect, rect);

		VuGfxUtil::IF()->fontDraw()->drawString(
			uiDrawParams.mDepth,
			fontEntry.font(),
			text,
			fontEntry.params(),
			rect,
			mStringFormat,
			alpha*mAlpha,
			mOffset*uiDrawParams.mLocalScale*uiDrawParams.mInvAuthScale
		);
	}
}
