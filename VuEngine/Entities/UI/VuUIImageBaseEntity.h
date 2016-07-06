//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Image Base class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/UI/VuUIPropertyUtil.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Properties/VuAssetProperty.h"

class VuScriptComponent;
class VuTransitionComponent;
class VuTexture;


class VuUIImageBaseEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUIImageBaseEntity();

protected:
	// event handlers
	virtual void					OnUITick(const VuParams &params);
	virtual void					OnUIDraw(const VuParams &params);

	VuRetVal						Show(const VuParams &params)		{ mbVisible = true; return VuRetVal(); }
	VuRetVal						Hide(const VuParams &params)		{ mbVisible = false; return VuRetVal(); }
	VuRetVal						SetAlpha(const VuParams &params);

	virtual void					drawLayout(bool bSelected)	{ drawImage(1.0f); }
	virtual void					drawImage(float alpha);

	virtual VuTexture *				getTexture() const = 0;

	// components
	VuTransitionComponent			*mpTransitionComponent;
	VuScriptComponent				*mpScriptComponent;

	// properties
	bool							mbVisible;
	VuColor							mColor;
	VuUIRectProperties				mRect;
	VuUIRectProperties				mSrcRect;
	float							mRotation;
	bool							mFlipX;
	bool							mFlipY;
	VuUIAnchorProperties			mAnchor;

	float							mAlpha;
};
