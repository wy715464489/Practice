//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Image Base class
// 
//*****************************************************************************

#include "VuUIImageBaseEntity.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/2dLayout/Vu2dLayoutComponent.h"
#include "VuEngine/Components/VuTransitionComponent.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Gfx/VuGfxUtil.h"


IMPLEMENT_RTTI(VuUIImageBaseEntity, VuEntity);


//*****************************************************************************
VuUIImageBaseEntity::VuUIImageBaseEntity():
	mbVisible(true),
	mColor(255,255,255),
	mRect(40,40,20,20),
	mSrcRect(0,0,1,1),
	mRotation(0),
	mFlipX(false),
	mFlipY(false),
	mAlpha(1.0f)
{
	// properties
	addProperty(new VuBoolProperty("Visible", mbVisible));
	addProperty(new VuColorProperty("Color", mColor));
	ADD_UI_RECT_PROPERTIES(getProperties(), mRect, "");
	ADD_UI_RECT_PROPERTIES(getProperties(), mSrcRect, "Src");
	addProperty(new VuAngleProperty("Rotation", mRotation));
	addProperty(new VuBoolProperty("FlipX", mFlipX));
	addProperty(new VuBoolProperty("FlipY", mFlipY));
	ADD_UI_ANCHOR_PROPERTIES(getProperties(), mAnchor, "");

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
	addComponent(new Vu2dLayoutComponent(this, &VuUIImageBaseEntity::drawLayout));
	addComponent(mpTransitionComponent = new VuTransitionComponent(this));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(getComponent<VuScriptComponent>(), VuUIImageBaseEntity, Show);
	ADD_SCRIPT_INPUT_NOARGS(getComponent<VuScriptComponent>(), VuUIImageBaseEntity, Hide);
	ADD_SCRIPT_INPUT(getComponent<VuScriptComponent>(), VuUIImageBaseEntity, SetAlpha, VuRetVal::Void, VuParamDecl(1, VuParams::Float));

	// event handlers
	REG_EVENT_HANDLER(VuUIImageBaseEntity, OnUITick);
	REG_EVENT_HANDLER(VuUIImageBaseEntity, OnUIDraw);
}

//*****************************************************************************
void VuUIImageBaseEntity::OnUITick(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	float fdt = accessor.getFloat();

	mpTransitionComponent->tick(fdt);
}

//*****************************************************************************
void VuUIImageBaseEntity::OnUIDraw(const VuParams &params)
{
	if ( mbVisible )
	{
		drawImage(mpTransitionComponent->alpha());
	}
}

//*****************************************************************************
VuRetVal VuUIImageBaseEntity::SetAlpha(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	mAlpha = accessor.getFloat();

	return VuRetVal();
}

//*****************************************************************************
void VuUIImageBaseEntity::drawImage(float alpha)
{
	VuUIDrawParams uiDrawParams;
	VuUIDrawUtil::getParams(this, uiDrawParams);

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();
	float authAR = uiDrawParams.mAuthScale.mX/uiDrawParams.mAuthScale.mY;

	VuRect dstRect = uiDrawParams.transform(mRect);

	mAnchor.apply(dstRect, dstRect);

	VuVector3 vCenter(dstRect.getCenter().mX, dstRect.getCenter().mY, 0);
	VuMatrix mat = VuMatrix::identity();
	mat.translate(-vCenter);
	mat.scale(VuVector3(authAR, 1.0f, 1.0f));
	mat.rotateZ(mRotation);
	mat.scale(VuVector3(1.0f/authAR, 1.0f, 1.0f));
	mat.translate(vCenter);

	VuRect srcRect = mSrcRect;
	if ( mFlipX )
		srcRect.flipX();
	if ( mFlipY )
		srcRect.flipY();

	VuColor color = mColor;
	color.mA = (VUUINT8)VuRound(color.mA*alpha*mAlpha);

	pGfxUtil->pushMatrix(mat*pGfxUtil->getMatrix());
	{
		if(VuTexture *pTexture = getTexture())
		{
			pGfxUtil->drawTexture2d(uiDrawParams.mDepth, pTexture, color, dstRect, srcRect);
		}
		else
		{
			pGfxUtil->drawFilledRectangle2d(uiDrawParams.mDepth, color, dstRect);
		}
	}
	pGfxUtil->popMatrix();
}
