//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Screen class
// 
//*****************************************************************************

#include "VuUIScreenEntity.h"
#include "VuEngine/UI/VuUI.h"
#include "VuEngine/UI/VuUIDrawUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"


IMPLEMENT_RTTI(VuUIScreenEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIScreenEntity);


//*****************************************************************************
VuUIScreenEntity::VuUIScreenEntity() : VuEntity(CAN_HAVE_CHILDREN),
	mInputUtil(this),
	mFullScreenLayer(VuGfxSort::FSL_UI)
{
	// event handlers
	REG_EVENT_HANDLER(VuUIScreenEntity, DisableInput);
	REG_EVENT_HANDLER(VuUIScreenEntity, EnableInput);
}

//*****************************************************************************
void VuUIScreenEntity::onGameInitialize()
{
	mInputUtil.enable();
}

//*****************************************************************************
void VuUIScreenEntity::onGameRelease()
{
	mInputUtil.disable();
}

//*****************************************************************************
void VuUIScreenEntity::tick(float fdt, VUUINT32 padMask)
{
	// input
	mInputUtil.tick(padMask);

	// tick
	VuParams params;
	params.addFloat(fdt);
	handleEventRecursive("OnUITick", params);
}

//*****************************************************************************
void VuUIScreenEntity::draw()
{
	VuGfxSort::IF()->setFullScreenLayer(mFullScreenLayer);
	VuGfxSort::IF()->setViewport(0);
	VuGfxSort::IF()->setReflectionLayer(0);
	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_UI);

	// set up crop matrix and text scale
	VuGfxUtil::IF()->pushMatrix(VuUI::IF()->getCropMatrix());
	VuGfxUtil::IF()->pushTextScale(VuUI::IF()->getTextScale());

	// draw
	handleEventRecursive("OnUIDraw");

	// pop state
	VuGfxUtil::IF()->popTextScale();
	VuGfxUtil::IF()->popMatrix();
}
