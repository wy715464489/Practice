//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Helpers for complex UI entity properties.
// 
//*****************************************************************************

#pragma once

#include "VuUIAnchor.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Util/VuStringFormat.h"
#include "VuEngine/Properties/VuProperties.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"

class VuProperties;
class VuTextureAsset;
class VuTexture;

class VuUIRectProperties : public VuRect
{
public:
	VuUIRectProperties() : VuRect(0,0,0,0) {}
	VuUIRectProperties(float x, float y, float w, float h) : VuRect(x,y,w,h) {}

	#define ADD_UI_RECT_PROPERTIES(properties, rect, name)					\
	{																		\
		properties.add(new VuFloatProperty(name "X", rect.mX));				\
		properties.add(new VuFloatProperty(name "Y", rect.mY));				\
		properties.add(new VuFloatProperty(name "Width", rect.mWidth));		\
		properties.add(new VuFloatProperty(name "Height", rect.mHeight));	\
	}
};


class VuUIStringFormatProperties : public VuStringFormat
{
public:
	static VuStaticIntEnumProperty::Choice sOptAlignH[];
	static VuStaticIntEnumProperty::Choice sOptAlignV[];

	#define ADD_UI_STRING_FORMAT_PROPERTIES(properties, stringFormat, name)																		\
	{																																			\
		properties.add(new VuStaticIntEnumProperty(name "Horizontal Alignment", stringFormat.mAlignH, VuUIStringFormatProperties::sOptAlignH));	\
		properties.add(new VuStaticIntEnumProperty(name "Vertical Alignment", stringFormat.mAlignV, VuUIStringFormatProperties::sOptAlignV));	\
		properties.add(new VuBoolProperty(name "Clip", stringFormat.mClip));																	\
		properties.add(new VuBoolProperty(name "Wordbreak", stringFormat.mWordbreak));															\
		properties.add(new VuBoolProperty(name "ShrinkToFit", stringFormat.mShrinkToFit));														\
	}
};

class VuUIImageProperties
{
public:
	VuUIImageProperties();

	void			addProperties(VuProperties &properties, const char *strName);

	VuTextureAsset	*getTextureAsset() const;
	VuTexture		*getTexture() const;

private:
	std::string						mTextureAssetName;
	VuAssetProperty<VuTextureAsset>	*mpTextureAssetProperty;
};

class VuUIAnchorProperties : public VuUIAnchor
{
public:
	static VuStaticIntEnumProperty::Choice sOptAnchorH[];
	static VuStaticIntEnumProperty::Choice sOptAnchorV[];

	#define ADD_UI_ANCHOR_PROPERTIES(properties, anchor, name)																		\
	{																																\
		properties.add(new VuStaticIntEnumProperty(name "Horizontal Anchor", anchor.mAnchorH, VuUIAnchorProperties::sOptAnchorH));	\
		properties.add(new VuFloatProperty(name "Horizontal Ratio", anchor.mRatioH));												\
		properties.add(new VuStaticIntEnumProperty(name "Vertical Anchor", anchor.mAnchorV, VuUIAnchorProperties::sOptAnchorV));	\
		properties.add(new VuFloatProperty(name "Vertical Ratio", anchor.mRatioV));													\
	}
};