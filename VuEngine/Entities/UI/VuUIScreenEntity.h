//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Screen class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/UI/VuUIInputUtil.h"


class VuUIScreenEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUIScreenEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

	virtual void		tick(float fdt, VUUINT32 padMask = 0xff);
	virtual void		draw();

	void				setFullScreenLayer(VUUINT fsl) { mFullScreenLayer = fsl; }
	void				setPriority(VUUINT32 priority) { mInputUtil.setPriority(priority); }

private:
	// event handlers
	void				DisableInput(const VuParams &params) { mInputUtil.disable(); }
	void				EnableInput(const VuParams &params) { mInputUtil.enable(); }

	VuUIInputUtil		mInputUtil;
	VUUINT				mFullScreenLayer;
};
