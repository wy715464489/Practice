//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Line Entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/UI/VuUIPropertyUtil.h"


class VuUILineEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUILineEntity();

	void		drawLayout(bool bSelected) { drawLine(1.0f); }
	void		drawLine(float alpha);

protected:
	// event handlers
	void					OnUIDraw(const VuParams &params);

	// properties
	VuVector2				mPoint0;
	VuVector2				mPoint1;
	VuColor					mColor;
	VuUIAnchorProperties	mAnchor;
};

IMPLEMENT_RTTI(VuUILineEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUILineEntity);


//*****************************************************************************
VuUILineEntity::VuUILineEntity():
	mPoint0(0,0),
	mPoint1(0,0),
	mColor(255,255,255)
{
	// properties
	addProperty(new VuVector2Property("Point 0", mPoint0));
	addProperty(new VuVector2Property("Point 1", mPoint1));
	addProperty(new VuColorProperty("Color", mColor));
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");

	// components
	addComponent(new Vu2dLayoutComponent(this, &VuUILineEntity::drawLayout));

	// event handlers
	REG_EVENT_HANDLER(VuUILineEntity, OnUIDraw);
}

//*****************************************************************************
void VuUILineEntity::OnUIDraw(const VuParams &params)
{
	float alpha = 1.0f;

	drawLine(alpha);
}

//*****************************************************************************
void VuUILineEntity::drawLine(float alpha)
{
	VuUIDrawParams uiDrawParams;
	VuUIDrawUtil::getParams(this, uiDrawParams);

	VuVector2 p0 = uiDrawParams.transform(mPoint0);
	VuVector2 p1 = uiDrawParams.transform(mPoint1);

	mAnchor.apply(p0, p0);
	mAnchor.apply(p1, p1);

	VuGfxUtil::IF()->drawLine2d(uiDrawParams.mDepth, mColor, p0, p1);
}
