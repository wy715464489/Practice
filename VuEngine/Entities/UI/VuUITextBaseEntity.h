//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Text Base class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/UI/VuUIPropertyUtil.h"
#include "VuEngine/Entities/VuEntity.h"

class VuScriptComponent;


class VuUITextBaseEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUITextBaseEntity();

	virtual const char				*getText() = 0;

protected:
	// event handlers
	virtual void					OnUIDraw(const VuParams &params);

	VuRetVal						Show(const VuParams &params)		{ mbVisible = true; return VuRetVal(); }
	VuRetVal						Hide(const VuParams &params)		{ mbVisible = false; return VuRetVal(); }
	VuRetVal						SetAlpha(const VuParams &params);

	virtual void					drawLayout(bool bSelected);
	virtual void					drawText(const char *text, float alpha);

	VuVector2						measureString();

	// components
	VuScriptComponent				*mpScriptComponent;

	// properties
	bool							mbVisible;
	VuUIRectProperties				mRect;
	std::string						mFont;
	VuUIStringFormatProperties		mStringFormat;
	VuUIAnchorProperties			mAnchor;

	VuVector2						mOffset; // may be used by derived classes to offset text
	float							mAlpha;
};
