//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Util
// 
//*****************************************************************************

#pragma once

class VuEntity;


namespace VuUIUtil
{
	void	startTransitionIn(VuEntity *pEntity);
	void	startTransitionOut(VuEntity *pEntity);
	bool	tickTransition(VuEntity *pEntity, float fdt);
}

