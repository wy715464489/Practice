//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Timer Service
// 
//*****************************************************************************

#include "VuTimerService.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/VuPfxEntity.h"
#include "VuEngine/Pfx/VuPfx.h"


//*****************************************************************************
VuTimerService::VuTimerService()
{
}

//*****************************************************************************
void VuTimerService::init(Callback *pCB, VUUINT32 id, float duration)
{
	mpCB = pCB;
	mID = id;
	mTimeRemaining = duration;
}

//*****************************************************************************
bool VuTimerService::tick(float fdt)
{
	mpCB->onTimerServiceTick(mID, fdt);

	if ( mTimeRemaining >= 0.0f )
	{
		mTimeRemaining -= fdt;
		if ( mTimeRemaining < 0.0f )
		{
			mpCB->onTimerServiceExpired(mID);
			return false;
		}
	}

	return true;
}
